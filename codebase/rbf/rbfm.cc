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
	
/*	int numAttr = recordDescriptor.size();	//get the number of attributes to find the number of bytes needed to describe the null attributes
	int numNullBytes = ceil((float)(numAttr)/8);		//get the number of null bytes
	vector<char> nullBytes;	//stores the bytes representing null attributes
	vector<bool> nullAttr;	//stores the bits representing null attributes after they've been split apart from bytes.

	//make the vectors large enough
	nullBytes.resize(numNullBytes*10);	
	nullAttr.resize(numNullBytes*8);
	
	//extracting the null bits
	for(int i = 0; i < numNullBytes; i++)
	{
		char* aByte = (char *) malloc(sizeof(char*)+1);
		int curBit = 0x80; //for tracking the current bit when anding it with the nullBytes		
		aByte = (char *) data; 

		void* aByte2 = malloc(numNullBytes);

		memcpy(aByte2, data, numNullBytes);

		for(int j = 0; j<8; j++)
		{
			int bitwise = ((*((char*)aByte2)) & curBit);
			if (curBit == bitwise)	//if the bit we're looking at is 1, then the value of attribute j is null
			{
				nullAttr[j] = 1;
			}
			else nullAttr[j] = 0;
			curBit = curBit>>1;	//double current bit, making it look like the next bit.
		}
	}
*/
	//round 2, fight
	//
	//
	int numAttr = recordDescriptor.size();		//get the number of attributes to find the number of bytes needed to describe the null attributes of the record
	int numNullBytes = ceil((float)(numAttr)/8.0);	//get the number of null bytes
	char* nullBytes;	//store the bytes representing null attributes;
	vector<bool> nullAttr;	//store the bits representing null attributes after being split apart.

	//allocate some space for bytes and bits
	nullBytes = (char*) malloc(numNullBytes);
	nullAttr.resize(numNullBytes*8);

	memcpy((void*)nullBytes, data, numNullBytes); //copy the number of null bytes from data into our nullBytes char array, filling nullBytes

	for(int i = 0; i < numNullBytes; i++)
	{	
		//curBit starts at 128, 10000000 in binary, for ANDing with the bytes later
		//decreases by a factor of 2 each time, 01000000, 00100000 and so on.
		int curBit = 0x80;

		for(int j = 0; j < 8; j++)	
		{
			if(curBit == (curBit & *(nullBytes + i*sizeof(char))))
			{
				nullAttr[j] = 1;
			}
			else nullAttr[j] = 0;
			
			cout<<curBit<<" "<<nullAttr[j]<<" "<<(curBit & *(nullBytes /*+ i*sizeof(char)*/))<<endl;
			curBit = curBit>>1;
			
		}
	}	


	
	//print rest.
	unsigned auxOffset = numNullBytes*sizeof(char);
	for (int i = 0; i < numAttr; i++)
	{
		string attrName = recordDescriptor[i].name;
		unsigned length = recordDescriptor[i].length;	//length is in # of bytes
		unsigned offset =  auxOffset;
			
		cout<<attrName<<": ";
		if (nullAttr[i] == 1)
		{
			cout<<"NULL ";
		}
		else switch(recordDescriptor[i].type)
		{
			case 0 : 	int dataInt;
					cout<<dataInt<<" ";
					break;
			case 1 :	float dataFloat;
					cout<<dataFloat<<" ";
					break;
			case 2 :		
					break;
		}
		auxOffset = length*sizeof(char) + auxOffset;
	} 
	cout<<endl;
    return 0;
}
