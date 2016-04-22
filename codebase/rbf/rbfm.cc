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


//Takes in a bytes to be scanned, a pointer to said bytes, and a vector of bools
//all data in the vector of bools will be clobbered
//the vector need not be initialized, but can be.
//the bytes will be scanned and the value of each bit will be placed into the vector of bools with the most significant bit 
//	occupying vector position 0 and the least significant bit occupying vector position ((numNullBytes * 8) - 1)
RC getNullAttr(const int numNullBytes, const void* nullBytes, vector<bool>& nullBits)
{
	nullBits.resize(numNullBytes*8);

	for(int i = 0; i < numNullBytes; i++)
	{	
		//curBit starts at 128, 10000000 in binary, for ANDing with the bytes later
		//decreases by a factor of 2 each time, 01000000, 00100000 and so on.
		//when nullBytes is ANDed with curBit, it determines if the bit we're interested in is currently on and if so stores that truth in the nullAttr array
		int curBit = 0x80;

		for(int j = 0; j < 8; j++)	
		{
			if(curBit == (curBit & *((char*)nullBytes + i*sizeof(char))))
			{
				nullBits[j * (i+1)] = 1;
			}
			else nullBits[j * (i+1)] = 0;	
			curBit = curBit>>1;		
		}
	}	
	return 0;
}

//needs a pointer to the date
//needs an offset from the beginning of the data to the start of the field
//needs a length of the field
//
//returns a void pointer to a field that contains the field data and only the field you are looking for.
//must be recast as the field you're looking for in order to use
//
//must be called twice on a VarChar. Once to get the 4 byte int the preceeds the data
//	and a second time using that int as the length of the var char and with an offset that is sizeof(int) greater than the initial offset
//	ex:
//		getField(data, offset, sizeof(int), VarCharSize);
//		getField(data, offset+sizeof(int), VarCharSize, VarCharData);
//	see printRecord switch case 2 for further example
RC getField(const void* data, const int offset, const int length, void*& field)
{

	field = malloc(length * sizeof(char));
	memcpy(field, (void*)(((char*)data)+offset), length * sizeof(char));

	return 0;
} 


RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	
	int numPages = -1;	//to find last page in the file
	RC err = 0;			//to store most recent error
	
	int numAttr = recordDescriptor.size();	//get the number of attributes to find the number of bytes needed to describe the null attributes
	int numNullBytes = ceil((float)numAttr/8.0);		//get the number of null bytes
	void* nullBytes;	
	vector<bool> nullAttr;
	
	err = getField(data, 0, numNullBytes, nullBytes); //get nullBytes
	if (err != 0)
	{
		return (1);	//return error, bad data grab, currently no way for getting here. if you receive this error, something serious has broken
	}
	err = getNullAttr(numNullBytes, nullBytes, nullAttr);
	if (err != 0)
	{
		return (2);	//return error, bad null bytes, currently no way of getting here either. if you receive this error, something serious has broken
	}

	unsigned offset = numNullBytes*sizeof(char);
	for (int i = 0; i < numAttr; i++)
	{
		string attrName = recordDescriptor[i].name;
		unsigned length = 4;	//I know there's a length in the recordDescriptor, but we always grab 4 bytes in the first grab of this program for varChar, int, and float
			 //length = recordDescriptor[i].length;	//length is in # of bytes
		cout<<attrName<<": ";
		//if null, indicate that in the record
		if (nullAttr[i] == 1)
		{
			/* stuff to include information about null in written record,
 				likely something in the directory*/
		}
		//else, print the value
		else 
		{
			void* dataField;
			getField(data, offset, length, dataField);
			switch(recordDescriptor[i].type)
			{
				case 0 :	
					{
						/*stuff to put an int into the record*/
					}
						break;
			
				case 1 :	
					{
						/*stuff to put a float into the record*/	
					}	
						break;
			
				case 2 :	
					{
						void* dataVarCharLength = dataField;
						string dataVarChar;

						getField(data, offset+sizeof(int), *(int*)dataVarCharLength, dataField);
						dataVarChar.assign((char*)dataField, *(int*)dataVarCharLength);		

						/*stuff to put a var char into the record*/

						//the total length is not just the initial 4 bytes, but also the length of the VarChar
						length = length + *(int*)dataVarCharLength;
						free (dataVarCharLength);
					}
						break;
			}
			free (dataField);
			offset = offset + length;
		}
	} 

	
	
	
//	rid.pageNum = 	//something
//	rid.slotNum = 	//something so that the record can be found again in the future
      return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    // to be implemented in project 1
    //note, we need to be able to read a field in the record in O(1) time, so probably going to need to create and maintain a directory to manage this as the VarChars will
    //		cause problems with other solutions
    //each record should probably begin with an array of ints that contain offsets from a local point (the front of the array or the first field?) to the beginning 
    //		of the field.
    //If we don't also have information about the recordDescriptor, then we'll need to also keep an offset to the end of the field so that we don't overrun the end of the field
	return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
	RC err = 0;			//to store most recent error
	
	int numAttr = recordDescriptor.size();		//get the number of attributes to find the number of bytes needed to describe the null attributes of the record
	int numNullBytes = ceil((float)(numAttr)/8.0);	//get the number of null bytes
	void* nullBytes;	//store the bytes representing null attributes;
	vector<bool> nullAttr;	//store the bits representing null attributes after being split apart.

	//allocate some space for bytes and bits and grab the null attributes
	nullAttr.resize(numNullBytes*8);
	getField(data, 0, numNullBytes, nullBytes);
	getNullAttr(numNullBytes, nullBytes, nullAttr);
	
	//print rest.
	unsigned offset = numNullBytes*sizeof(char);
	for (int i = 0; i < numAttr; i++)
	{
		string attrName = recordDescriptor[i].name;
		unsigned length = 4;	//I know there's a length in the recordDescriptor, but we always grab 4 bytes in the first grab of this program for varChar, int, and float
			 //length = recordDescriptor[i].length;	//length is in # of bytes
		cout<<attrName<<": ";
		//if null, print null
		if (nullAttr[i] == 1)
		{
			cout<<"NULL ";
		}
		//else, print the value
		else 
		{
			void* dataField;
			getField(data, offset, length, dataField);
			switch(recordDescriptor[i].type)
			{
				case 0 :	
					{
						cout<<*(int*)dataField<<" ";
					}
						break;
			
				case 1 :	
					{	
						cout<<*(float*)dataField<<" ";
					}	
						break;
			
				case 2 :	
					{
						void* dataVarCharLength = dataField;
						string dataVarChar;

						getField(data, offset+sizeof(int), *(int*)dataVarCharLength, dataField);
						dataVarChar.assign((char*)dataField, *(int*)dataVarCharLength);		
						cout<<dataVarChar<<" ";

						//the total length is not just the initial 4 bytes, but also the length of the VarChar
						length = length + *(int*)dataVarCharLength;
						free (dataVarCharLength);
					}
						break;
			}
			free (dataField);
			offset = offset + length;
		}
	} 
	cout<<endl;

	//garbage collection
	free (nullBytes);
    return 0;
}
