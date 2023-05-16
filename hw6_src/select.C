#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int recordlen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;
    AttrDesc attrDesc;
    const char* filter;
    // array of attrDesc to hold projection decsription
    AttrDesc* projNamesDesc = new AttrDesc[projCnt];
	Status status;
    // retreving record total length
	int recordlen = 0;
    int idx = 0;
    while(idx < projCnt){
        if ((status = attrCat->getInfo(projNames[idx].relName, projNames[idx].attrName, projNamesDesc[idx])) == OK){
			recordlen += projNamesDesc[idx].attrLen;
		}else{
			return status;
		}
        idx ++;        
        
    }
    // for (int i = 0; i < projCnt; i++) {
    //     if ((status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projNamesDesc[i])) == OK){
	// 		recordlen += projNamesDesc[i].attrLen;
	// 	}else{
	// 		return status;
	// 	}
    // }

    if (attr == NULL){
        // Simply select all
		// book keeping to get all
        strcpy(attrDesc.relName, projNames[0].relName);
        strcpy(attrDesc.attrName, projNames[0].attrName);
        attrDesc.attrOffset = 0;
        attrDesc.attrLen = 0;
        attrDesc.attrType = STRING;
        filter = NULL;

        // building table after book keeping
        if ((status = ScanSelect(result, projCnt, projNamesDesc, &attrDesc, EQ, filter, recordlen)) != OK) return status;
    }else {
		// where clause needed
        if((status = attrCat->getInfo(string(attr->relName), string(attr->attrName), attrDesc)) != OK) return status;

		// correponding type cast
        if (attr->attrType == FLOAT){
			float floatCast = atof(attrValue);
            filter = (char*)&floatCast;
        }else if (attr->attrType == INTEGER){
			int intCast = atoi(attrValue);
            filter = (char*)&intCast;
        }else if (attr->attrType == STRING){
            filter = attrValue;
        }

        // building table after book keeping
        if ((status = ScanSelect(result, projCnt, projNamesDesc, &attrDesc, op, filter, recordlen)) != OK) return status;
    }
    return OK;
}


const Status ScanSelect(const string & result, 
#include "stdio.h"
#include "stdlib.h"
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int recordlen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
	Status status;

    
    // start to init a pointer
    char output[recordlen];
    // init record to be inserted

    HeapFileScan relScan(attrDesc->relName, status);
    if((status = relScan.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype) attrDesc->attrType, filter, op))!=OK)return status;

    RID relationRID;
    Record relRec;
 
    for (status = relScan.scanNext(relationRID); status == OK; status = relScan.scanNext(relationRID)){
        // find all records
        if((status = relScan.getRecord(relRec)) != OK) return status;

        // copying data into output record for future insert
        int offset = 0;
        int index = 0;
        while(index < projCnt){
            memcpy(output + offset, (char *)relRec.data + projNames[index].attrOffset, projNames[index].attrLen);
            offset += projNames[index].attrLen;
            index++;
        }
		Record outputRecord;
        RID outRID;
		outputRecord.length = recordlen;
    	outputRecord.data = (void *) output;
        InsertFileScan resultRel(result, status);
        if((status = resultRel.insertRecord(outputRecord, outRID)) != OK) return status;
    }
    return OK;
}

