/**
 * @file buf.C
 *
 * @brief Responsible for buffer management and finding avaliable base on the clock algorithm
 *
 * @author Matthew Liu, Wei-Jen Chen， Leqi Xu
 * Contact: mliu362@wisc.edu, wchen@wisc.edu, lxu284@wisc.edu
 *
 */
#include <iostream>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include "page.h"
#include "buf.h"
#include "error.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    
    
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}

const Status BufMgr::allocBuf(int & frame) 
{
  Status status = OK;
  // the count of times for clockHand being advanced
  int count = 0;
  bool allocated = false;
  // traverse two times the clock to avoid false validation of freee frames
  while(count < (int)numBufs * 2){
    if(bufTable[clockHand].valid == true){
      if(bufTable[clockHand].refbit == false){
          if(bufTable[clockHand].pinCnt > 0){
            // pinned, can't be used, advance the clock
            advanceClock();
            count++;
            continue;
          }else{
            if(bufTable[clockHand].dirty == false){  
                // simple remove with clean frames 
                frame = clockHand;
                hashTable->remove(bufTable[clockHand].file, bufTable[clockHand].pageNo);
                allocated = true;
                break;
              }else{
                //flushing modified page to disk
                if ((status = bufTable[clockHand].file->writePage(bufTable[clockHand].pageNo,&bufPool[clockHand]))!= OK){
                  return UNIXERR;
                }
                //updating info and clock
                bufStats.diskwrites++;
                bufStats.accesses++;
                hashTable->remove(bufTable[clockHand].file, bufTable[clockHand].pageNo);
                frame = clockHand;
                allocated = true;
                break;
            }
          }
      }else{
        // clear refbit
        bufTable[clockHand].refbit = false;
        advanceClock();
        count++;
        continue;
      }
    }else{
      frame = clockHand;
      allocated = true;
      break;
    }
  }
  //final status check after the loop
  if (!allocated){
    return BUFFEREXCEEDED;
  }else{
    return status;
  }
  
  //return status;
}

	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
  Status status = OK;
  int frame;
  
  if ((status = hashTable->lookup(file, PageNo, frame)) != OK){
    //Case: page not found in buffer pool
    //find an available buffer frame allow to put page in by call allocBuf()
    if ((status = allocBuf(frame)) != OK){
      return status;
    }  
    //reading page in to buffer pool
    if ((status = file->readPage(PageNo,&bufPool[frame])) != OK){
      return status;
    }
    bufStats.diskreads++;
    //updating metadata in hashTable
    if ((status = hashTable->insert(file, PageNo, frame)) != OK){
      return status;
    }
    //Calling Set() on the frame to set up
    bufTable[frame].Set(file, PageNo);
   
  }else{
    //Case: Page found in the buffer pool
    //settig refbit
    bufTable[frame].refbit=true;
    //updating pinCnt
    bufStats.accesses++;
    bufTable[frame].pinCnt++;
  }
  page = &bufPool[frame];
  return status;
}


const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
  Status status = OK;
  int frame;

  //finding the correspond PageNo frame in bufTable
  if ((status = hashTable->lookup(file, PageNo, frame)) != OK){
    return status;
  }

  //case when page in not pinned
  if (bufTable[frame].pinCnt == 0){
    status = PAGENOTPINNED;
    return status;
  } 

  //decrements the crresonding frame pinCnt 
  bufTable[frame].pinCnt--;

  //set dirty bit for dirty case
  if (dirty == true){
    bufTable[frame].dirty = true;
  }

  return status;
}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
  Status status = OK;
  //allocating empty page and checking status
  if ((status = file->allocatePage(pageNo)) != OK){
    return status;
  }
  int frame;
  //call allocBuf to get a buffer pool frame
  if ((status = allocBuf(frame)) != OK){
    return status;
  }

  bufStats.accesses++;
  //updating hashTable entry
  if((status = hashTable->insert(file, pageNo, frame)) != OK){
    return status;
  }

  //calling set() to set the table up properly
  page = &bufPool[frame];
  bufTable[frame].Set(file, pageNo);

  return status;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


