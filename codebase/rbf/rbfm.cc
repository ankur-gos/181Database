#include "rbfm.h"
#include "pfm.h"
#include <math.h>	//for ceiling()

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
    /*input notes
	We have a vector<attribute> which contains a collection of attribute structs
		attribute contains:
			string name
			attrType type
				enumerated as {int, real, varchar}
			attrLength length
				unsigned int 
	
	Then we have a pointer to data which is a pointer to a void type
		to dereference a void type, we need to recast it as what ever type it is, i.e. (int *)data
	Then we have an RID which contains		we return this
		unsigned pageNum	//int is assumed without another type
		unsigned slotNum	//int is assumed without another type
	*/
	
	/*insertRecord notes
	insert record into given file 
	assume input is error free
	to handle nulls, the first part of *data has n bytes for passing the null info
		n can be calculated through the this formula:
			ceil(# of fields in record/8)
			i.e. 5 fields is ceil(5/8) = 1 bytes
			we can determine # of fields by getting the size of the given vector
		the left most bit in the first byte corresponds to the first field
		the right most bit in the first byte corresponds to the eight field
		the left most bit in the first byte corresponds to the ninth field and so on
		if the corresponding bit to each field is set to 1, then the actual data does not contain any value for this field
		if there are three fields in a record and the second field contains null, the bit representation  in a byte is 01000000
		in addition, in the actual data, the incoming record contains the first and the third values only. 
		that is, the third field value is placed right after the first field value in thsi case.
	
	This format (null fields + actual data) is to be used for all record manipulation operations (unless otherwise stated)
		when a record is read, the first part of what you return should contain a null fields indicator
		
	File structure is a heap file, and you may use a system-sequenced file organization
		i.e. if the last page has enough space, insert a new record into this page.
		if not, find the first page with free space large enough to store the record, looking from the beginning of the file (not the page)
		
		An RID here is the record id which is used to uniquely identify records in a file
	*/
	
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
	
	int numAttr = recordDescriptor.size();	//get the number of attributes to find the number of bytes needed to describe the null attributes
	int numNullBytes = ceil(numAttr/8);		//get the number of null bytes
	vector<char> nullBytes;	//stores the bytes representing null attributes
	vector<bool> nullAttr;	//stores the bits representing null attributes after they've been split apart from bytes.
	
	//extracting the null bytes
	for(int i = 0; i < numNullBytes; i++)
	{
		nullBytes[i] = &(char*)data[i];			//get which attributes are null
	}
	
	//extracting the null bits
	for(int i = 0; i<numNullBytes; i++)
		int curBit = 1;
		for(char j = 0; j<8; j++)
		{
			if (curBit == nullBytes[i] & curBit)	//if the bit we're looking at is 1, then the value of attribute j is null
			{
				nullAttr[j] = 1;
			}
			curBit = curbit*2;	//double current bit, making it look like the next bit.
		}
	
	
	
    return 0;
}
