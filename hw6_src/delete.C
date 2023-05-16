#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{
	// part 6
	Status status;
    AttrDesc attrDesc;
    const char* filter;

    RID relationID;
    HeapFileScan *hfs = new HeapFileScan(relation, status);
    if (status != OK) return status; 

    if (attrName.length() != 0){
        // getting search info
        if ((status = attrCat->getInfo(relation, attrName, attrDesc)) != OK) return status;

        int intVal;
        float floatVal;
        // type conversion
        if (type == FLOAT){
            floatVal = atof(attrValue);
            filter = (char*)&floatVal;
        }else if (type  == INTEGER){
            intVal = atoi(attrValue);
            filter = (char*)&intVal;
        }else if (type == STRING){
            filter = attrValue;
        }

        // scan after book keeping
        if ((status = hfs->startScan(attrDesc.attrOffset, attrDesc.attrLen, type, filter, op)) != OK) return status;
      
        for (status = hfs->scanNext(relationID); status == OK; status = hfs->scanNext(relationID)){
			// actual deletion
            if ((status = hfs->deleteRecord())!= OK) return status;
        }

    }else{
        if ((status = hfs->startScan(0, 0, STRING, NULL, EQ)) != OK) return status;

        for (status = hfs->scanNext(relationID); status == OK; status = hfs->scanNext(relationID)){
			// actual deletion
            if ((status = hfs->deleteRecord())!= OK) return status;
        }
        
        return OK;
    }
    // end scan
    hfs->endScan();
    // deletion
    delete hfs;
    return OK;

}




