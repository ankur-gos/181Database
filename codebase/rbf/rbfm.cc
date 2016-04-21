#include "rbfm.h"
#include "pfm.h"
#include <math.h>	//for ceiling()
#include <cstring>	//for memcpy

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    PagedFileManager * _page_manager = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
    _page_manager->createFile(fileName);
	return 0;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    _page_manager->destroyFile(fileName);
    return 0;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    _page_manager->openFile(fileName, fileHandle);
    return 0;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    _page_manager->closeFile(fileHandle);
    return 0;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	
/*	int numPages = -1;	//to find last page in the file
	RC err = 0;			//to store most recent error
	
	int numAttr = recordDescriptor.size();	//get the number of attributes to find the number of bytes needed to describe the null attributes
	int numNullBytes = ceil(numAttr/8);		//get the number of null bytes
	int nullAttr = 	/**fix me		**/				//get which attributes are null
	
/*	int numPages = fileHandle->getNumberOfPages();
	int err = fileHandle->writePage(numPages-1, 	///How do I determine the slot the file is written to.
	if (err != 0)	
	{
		if (err == /**page full**///)	/*page full*/
/*		{
			/**need to scan for new location here**/	//scan for open slot elsewhere.
/*		}
		else return (1);	//return error code, for bad write
	}

	
	
	
	rid.pageNum = /**get page from somewhere**/	//something
/*	rid.slotNum = /**get slot from somewhere**/	//something so that the record can be found again in the future
      return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
	RC err = 0;			//to store most recent error
	
	int numAttr = recordDescriptor.size();		//get the number of attributes to find the number of bytes needed to describe the null attributes of the record
	int numNullBytes = ceil((float)(numAttr)/8.0);	//get the number of null bytes
	char* nullBytes;	//store the bytes representing null attributes;
	vector<bool> nullAttr;	//store the bits representing null attributes after being split apart.

	//allocate some space for bytes and bits
	nullBytes = (char*) malloc(numNullBytes);
	nullAttr.resize(numNullBytes*8);

	memcpy((void*)nullBytes, data, numNullBytes); //copy the number of null bytes from data into our nullBytes char array, filling nullBytes

	//grabbing null attributes
	for(int i = 0; i < numNullBytes; i++)
	{	
		//curBit starts at 128, 10000000 in binary, for ANDing with the bytes later
		//decreases by a factor of 2 each time, 01000000, 00100000 and so on.
		//when nullBytes is ANDed with curBit, it determines if the bit we're interested in is currently on and if so stores that truth in the nullAttr array
		int curBit = 0x80;

		for(int j = 0; j < 8; j++)	
		{
			if(curBit == (curBit & *(nullBytes + i*sizeof(char))))
			{
				nullAttr[j] = 1;
			}
			else nullAttr[j] = 0;	
			curBit = curBit>>1;		
		}
	}	


	
	//print rest.
	unsigned offset = numNullBytes*sizeof(char);
	for (int i = 0; i < numAttr; i++)
	{
		string attrName = recordDescriptor[i].name;
		unsigned length = recordDescriptor[i].length;	//length is in # of bytes
			
		cout<<attrName<<": ";
		if (nullAttr[i] == 1)
		{
			cout<<"NULL ";
		}
		else switch(recordDescriptor[i].type)
		{
			case 0 : 	int* dataInt;
					dataInt = (int*)malloc(sizeof(int));

					//this is stupidly complex, so here's what's supposed happening.
					//the pointer to the int is cast to a void pointer so it can be used by memcpy
					//the pointer to data is cast to a char so it can be offset and then recast as a void so that it can be used by memcpy 
					memcpy((void*)dataInt, (void*)(((char*)data)+offset), sizeof(int));
	
					
					cout<<*dataInt<<" ";
					
					offset = offset + length*sizeof(char);
					free (dataInt);
					break;
			
			case 1 :	float* dataFloat;
					dataFloat = (float*)malloc(length * sizeof(char));
					memcpy((void*)dataFloat, (void*)((char*)data+offset), length);
					cout<<*dataFloat<<" ";
					
					free (dataFloat);
					offset = length*sizeof(char) + offset;
					break;
			
			case 2 :	int* dataVarCharLength;
					string dataVarChar;

					dataVarCharLength = (int*) malloc (sizeof(int));

					//VAR CHAR IS NOT ACTUALLY AS LONG AS RECORDDESCRIPTOR.LENGTH() SAYS IT IS, that's just the max length. IT'S SPECIFIED. FUCK, so much time wasted!
					//THE FIRST 4 BYTES ARE THE LENGTH.
					memcpy((void*)dataVarCharLength,(void*)((char*)data+offset), sizeof(int));
					dataVarChar.assign(((const char*)data+offset+sizeof(int)),  *dataVarCharLength);		
					//seriously, fuck this was so annoying

					cout<<dataVarChar<<" ";
					offset = *dataVarCharLength + sizeof(int) + offset;
					break;
		}
	} 
	cout<<endl;

	//garbage collection
	free (nullBytes);
    return 0;
}
