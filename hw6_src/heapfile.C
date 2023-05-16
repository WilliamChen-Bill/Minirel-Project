#include "heapfile.h"
#include "error.h"

// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
  File *file;
  Status status;
  FileHdrPage *hdrPage;
  int hdrPageNo;
  int newPageNo;
  Page *newPage;

  // try to open the file. This should return an error
  status = db.openFile(fileName, file);
  if (status != OK)
  {

    // file does not exist, create and reopen
    if ((status = db.createFile(fileName)) != OK)
      return status;
    if ((status = db.openFile(fileName, file)) != OK)
      return status;
    // init header page
    if ((status = bufMgr->allocPage(file, hdrPageNo, newPage)) != OK)
      return status;
    hdrPage = (FileHdrPage *)newPage;

    for (int i = 0; i != fileName.size(); ++i)
    {
      hdrPage->fileName[i] = fileName[i];
    }

    // init empty data page
    if ((status = bufMgr->allocPage(file, newPageNo, newPage)) != OK)
      return status;

    newPage->init(newPageNo);
    // forward pointer to the next
    newPage->setNextPage(-1);

    // update stats
    hdrPage->firstPage = newPageNo;
    hdrPage->lastPage = newPageNo;
    hdrPage->pageCnt = 1;
    hdrPage->recCnt = 0;

    // unpin
    if ((status = bufMgr->unPinPage(file, hdrPageNo, true)) != OK)
      return status;
    if ((status = bufMgr->unPinPage(file, newPageNo, true)) != OK)
      return status;

    // writing initial content
    bufMgr->flushFile(file);
    db.closeFile(file);

    return OK;
  }

  return (FILEEXISTS);
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
  return (db.destroyFile(fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string &fileName, Status &returnStatus)
{
  Status status;
  Page *pagePtr;

  // open the file and read in the header page and the first data page
  if ((status = db.openFile(fileName, filePtr)) == OK)
  {
    File *file = filePtr;

    int pageNo = -1;
    // getting page num
    if ((status = file->getFirstPage(pageNo)) != OK)
    {
      cerr << "failed calling getFirstPage to get number\n";
      returnStatus = status;
    }

    if ((status = bufMgr->readPage(file, pageNo, pagePtr)) != OK)
    {
      cerr << "readPage failed to get data\n";
      returnStatus = status;
    }

    headerPage = (FileHdrPage *)pagePtr;
    hdrDirtyFlag = false;
    headerPageNo = pageNo;
    

    int firstPageNo = curPageNo = headerPage->firstPage;
    if ((status = bufMgr->readPage(file, firstPageNo, pagePtr)) != OK)
    {
      cerr << "readPage on data failed\n";
      returnStatus = status;
      return;
    }

    // setting current page
    curPage = pagePtr;
    curDirtyFlag = false;
    returnStatus = OK;
    curRec = NULLRID;
    return;
  }
  else
  {
    cerr << "open of heap file failed\n";
    returnStatus = status;
    return;
  }
}

// the destructor closes the file
HeapFile::~HeapFile()
{
  Status status;
  // cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

  // see if there is a pinned data page. If so, unpin it
  if (curPage != NULL)
  {
    status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
    curPage = NULL;
    curPageNo = 0;
    curDirtyFlag = false;
    if (status != OK)
      cerr << "error in unpin of date page\n";
  }

  // unpin the header page
  status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
  if (status != OK)
    cerr << "error in unpin of header page\n";

  // status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
  // if (status != OK) cerr << "error in flushFile call\n";
  // before close the file
  status = db.closeFile(filePtr);
  if (status != OK)
  {
    cerr << "error in closefile call\n";
    Error e;
    e.print(status);
  }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

const Status HeapFile::getRecord(const RID &rid, Record &rec)
{
  Status status;

  if ((curPage != NULL) && curPageNo == rid.pageNo)
  {
    // correct page pinned
    curRec = rid;
    status = curPage->getRecord(curRec, rec);
    return status;
  }
  else
  {
    // wrong page pinned
    if ((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) != OK)
    {
      curPage = NULL;
      curPageNo = 0;
      curDirtyFlag = false;

      return status;
    }

    curPageNo = rid.pageNo;
    curDirtyFlag = false;
    curRec = rid;

    // read the page into the buffer pool
    if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK)
      return status;
    status = curPage->getRecord(curRec, rec);
    return status;
  }
}

HeapFileScan::HeapFileScan(const string &name,
                           Status &status) : HeapFile(name, status)
{
  filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
                                     const int length_,
                                     const Datatype type_,
                                     const char *filter_,
                                     const Operator op_)
{
  if (!filter_)
  { // no filtering requested
    filter = NULL;
    return OK;
  }

  if ((offset_ < 0 || length_ < 1) ||
      (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
      (type_ == INTEGER && length_ != sizeof(int) || type_ == FLOAT && length_ != sizeof(float)) ||
      (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
  {
    return BADSCANPARM;
  }

  offset = offset_;
  length = length_;
  type = type_;
  filter = filter_;
  op = op_;

  return OK;
}

const Status HeapFileScan::endScan()
{
  Status status;
  // generally must unpin last page of the scan
  if (curPage != NULL)
  {
    status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
    curPage = NULL;
    curPageNo = 0;
    curDirtyFlag = false;
    return status;
  }
  return OK;
}

HeapFileScan::~HeapFileScan()
{
  endScan();
}

const Status HeapFileScan::markScan()
{
  // make a snapshot of the state of the scan
  markedPageNo = curPageNo;
  markedRec = curRec;
  return OK;
}

const Status HeapFileScan::resetScan()
{
  Status status;
  if (markedPageNo != curPageNo)
  {
    if (curPage != NULL)
    {
      status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
      if (status != OK)
        return status;
    }
    // restore curPageNo and curRec values
    curPageNo = markedPageNo;
    curRec = markedRec;
    // then read the page
    status = bufMgr->readPage(filePtr, curPageNo, curPage);
    if (status != OK)
      return status;
    curDirtyFlag = false; // it will be clean
  }
  else
    curRec = markedRec;
  return OK;
}

const Status HeapFileScan::scanNext(RID &outRid)
{
  Status status = OK;
  RID nextRid;
  RID tmpRid;
  int nextPageNo;
  Record rec;
  // Already EOF
  if (curPageNo < 0)
    return FILEEOF;

  // firstpage not init
  if (!curPage)
  {
    // need to get the first page of the file
    curPageNo = headerPage->firstPage;

    // read the first page of the file
    status = bufMgr->readPage(filePtr, curPageNo, curPage);
    curRec = NULLRID;
    curDirtyFlag = false;
    if (status != OK)
      return status;
    else
    {
      // Get first record
      status = curPage->firstRecord(tmpRid);
      curRec = tmpRid;

      // case of empty first page
      if (status == NORECORDS)
      {
        if ((status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag)) != OK)
          return status;
        curPageNo = -1;
        curPage = NULL;
        return FILEEOF;
      }
      // move the pointer
      if ((status = curPage->getRecord(tmpRid, rec)) != OK)
        return status;

      // checking matches
      if (matchRec(rec) == true)
      {
        outRid = tmpRid;
        return OK;
      }
    }
  }
  // pinned page check for records or go to next page
  // variable for constant loop
  int i = 1;
  while (i == 1)
  {
    // getting next record
    // case when cannot get next record
    if ((status = curPage->nextRecord(curRec, nextRid)) != OK)
    {
      while ((status == NORECORDS) || (status == ENDOFPAGE))
      {
        // get the page number of next
        status = curPage->getNextPage(nextPageNo);
        // EOF
        if (nextPageNo == -1)
        {
          return FILEEOF;
        }

        // unpin page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPageNo = -1;
        curPage = NULL;
        if (status != OK)
          return status;

        // read next page
        curDirtyFlag = false;
        curPageNo = nextPageNo;
        if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK)
          return status;

        status = curPage->firstRecord(curRec);
      }
    }
    else
    {
      // move on within record
      curRec = nextRid;
    }
    // case when just valid
    if ((status = curPage->getRecord(curRec, rec)) != OK);
    // check for matches
    if (matchRec(rec) == true)
    {
      outRid = curRec;
      return OK;
    }
  }
}

// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page

const Status HeapFileScan::getRecord(Record &rec)
{
  return curPage->getRecord(curRec, rec);
}

// delete record from file.
const Status HeapFileScan::deleteRecord()
{
  Status status;

  // delete the "current" record from the page
  status = curPage->deleteRecord(curRec);
  curDirtyFlag = true;

  // reduce count of number of records in the file
  headerPage->recCnt--;
  hdrDirtyFlag = true;
  return status;
}

// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
  curDirtyFlag = true;
  return OK;
}

const bool HeapFileScan::matchRec(const Record &rec) const
{
  // no filtering requested
  if (!filter)
    return true;

  // see if offset + length is beyond end of record
  // maybe this should be an error???
  if ((offset + length - 1) >= rec.length)
    return false;

  float diff = 0; // < 0 if attr < fltr
  switch (type)
  {

  case INTEGER:
    int iattr, ifltr; // word-alignment problem possible
    memcpy(&iattr,
           (char *)rec.data + offset,
           length);
    memcpy(&ifltr,
           filter,
           length);
    diff = iattr - ifltr;
    break;

  case FLOAT:
    float fattr, ffltr; // word-alignment problem possible
    memcpy(&fattr,
           (char *)rec.data + offset,
           length);
    memcpy(&ffltr,
           filter,
           length);
    diff = fattr - ffltr;
    break;

  case STRING:
    diff = strncmp((char *)rec.data + offset,
                   filter,
                   length);
    break;
  }

  switch (op)
  {
  case LT:
    if (diff < 0.0)
      return true;
    break;
  case LTE:
    if (diff <= 0.0)
      return true;
    break;
  case EQ:
    if (diff == 0.0)
      return true;
    break;
  case GTE:
    if (diff >= 0.0)
      return true;
    break;
  case GT:
    if (diff > 0.0)
      return true;
    break;
  case NE:
    if (diff != 0.0)
      return true;
    break;
  }

  return false;
}

InsertFileScan::InsertFileScan(const string &name,
                               Status &status) : HeapFile(name, status)
{
  // Do nothing. Heapfile constructor will bread the header page and the first
  //  data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
  Status status;
  // unpin last page of the scan
  if (curPage != NULL)
  {
    status = bufMgr->unPinPage(filePtr, curPageNo, true);
    curPage = NULL;
    curPageNo = 0;
    if (status != OK)
      cerr << "error in unpin of data page\n";
  }
}

// Insert a record into the file
const Status InsertFileScan::insertRecord(const Record &rec, RID &outRid)
{
  Page *newPage;
  int newPageNo;
  Status status, unpinstatus;
  RID rid;

  // check for very large records
  if ((unsigned int)rec.length > PAGESIZE - DPFIXED)
  {
    // will never fit on a page, so don't even bother looking
    return INVALIDRECLEN;
  }

  if (curPage == NULL)
  {
    // make the last page the current page
    curPageNo = headerPage->lastPage;
    // read it into the buffer
    if ((status = bufMgr->readPage(filePtr, curPageNo, curPage)) != OK)
      return status;
  }

  // add record
  // Case can't allocate
  if ((status = curPage->insertRecord(rec, rid)) != OK)
  {
    // alloc new page since its full
    if ((status = bufMgr->allocPage(filePtr, newPageNo, newPage)) != OK)
      return status;

    // init empty
    newPage->init(newPageNo);

    // forward pointer
    if ((status = newPage->setNextPage(-1)) != OK)
      return status; // no next page

    // modify the header page content properly
    headerPage->pageCnt++;
    headerPage->lastPage = newPageNo;
    hdrDirtyFlag = true;

    // link up the new page appropriately
    // forward pointer
    if ((status = curPage->setNextPage(newPageNo)) != OK)
      return status;

    if ((status = bufMgr->unPinPage(filePtr, curPageNo, true)) != OK)
    {
      curPage = NULL;
      curPageNo = -1;
      curDirtyFlag = false;

      // unpin page
      unpinstatus = bufMgr->unPinPage(filePtr, newPageNo, true);
      return status;
    }

    // make the current page to be the newly allocated page
    curPageNo = newPageNo;
    curPage = newPage;

    // try to insert the record
    if ((status = curPage->insertRecord(rec, rid)) == OK)
    { // book keeping
      curDirtyFlag = true;
      hdrDirtyFlag = true;
      headerPage->recCnt++;
      outRid = rid;
      return status;
    }
    else
      return status;
  }
  else
  {
    // successful insert
    headerPage->recCnt++;
    curDirtyFlag = true;
    hdrDirtyFlag = true;
    outRid = rid;
    return status;
  }
}
