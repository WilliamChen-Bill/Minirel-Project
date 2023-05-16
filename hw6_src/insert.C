#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{
// part 6
    // start search by calling InsertFileScan
	Status status;
    InsertFileScan resultRel(relation, status);
    
	AttrDesc* relationAttr;
	int relationCount;
	// getting catelog info
    if ((status = attrCat->getRelInfo(relation, relationCount, relationAttr)) != OK) return status;
	
    int recordLen = 0;
	int index = 0;
	// Determine the combined size of records
	while(index<relationCount){
		recordLen += relationAttr[index].attrLen;
		index ++;	
	}
    char outputData[recordLen];
	// looking for matching attributes
    for (index = 0; index < attrCnt; index++) {
        for (int j = 0; j < relationCount; j++) {
			// not match move on
            if (strcmp(relationAttr[j].attrName, attrList[index].attrName) != 0) {
                continue;
            // matches
            }else{
                if (attrList[index].attrValue != NULL){
                    char* realVal;
                    int intVal;
                    float floatVal;
					// Type conversion
                    if (attrList[index].attrType == FLOAT){
                        floatVal = atof((char*)attrList[index].attrValue);
                        realVal = (char*)&floatVal;
                    }else if (attrList[index].attrType == INTEGER){
                        intVal = atoi((char*)attrList[index].attrValue);
                        realVal = (char*)&intVal;
                    }else if (attrList[index].attrType == STRING){
                        realVal = (char*) attrList[index].attrValue;
                    }
					// Copying matching data
                    memcpy(outputData + relationAttr[j].attrOffset, realVal, relationAttr[j].attrLen);
                }else{
					// Cannot allow NULL values
                    return ATTRTYPEMISMATCH;
                }
            }
        }
    }
	// Start insertion
    RID outputRID;
	Record outputRecord;
	outputRecord.length = recordLen;
    outputRecord.data = (void *) outputData;
	// Insertion status check
    if((status = resultRel.insertRecord(outputRecord, outputRID))!=OK){
		return status;
	}else{
		return OK;
	} 
}


