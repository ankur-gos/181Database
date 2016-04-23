#include "rbfm.h"
#include "pfm.h"

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
//`
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

RC _getField(const void* data, const int offset, const int length, void*& field)
{
    memcpy(field, (void*)(((char*)data)+offset), length * sizeof(char));

    return 0;
}


//reverse of getField
//puts a field into the given slot in data
RC putField(void*& data, const int offset, const int length, const void* field)
{

	memcpy((void*)(((char*)data)+offset), field, length * sizeof(char));

	return 0;
}

vector<int> RecordBasedFileManager::getPageViability(FileHandle &fileHandle, unsigned numberOfSlots, unsigned bytesNeeded, int pageNumber){
    vector<int> values;
    void *page = malloc(PAGE_SIZE);
    memset(page, 0, PAGE_SIZE);
    fileHandle.readPage(pageNumber, page);
    void *slot = malloc(sizeof(int));
    int slots[100];
    // Page table is 400 bytes
    for(int i = 0; i < 100; i++){
        getField(page, 3696 + i*sizeof(int), sizeof(int), slot);
        slots[i] = *(int*)slot;
    }
    int consecutiveSlotCount = 0;
    int available = 0;
    int first = 1;
    int slot_location = 0;
    for(int i = 0; i < 100; i++){
        if(slots[i] == -2){
            if(first){
                slot_location = i + 1;
            }
            consecutiveSlotCount++;
        } else{
            first = 1;
            consecutiveSlotCount = 0;
        }

        if(consecutiveSlotCount == numberOfSlots + 2){
            available = 1;
            break;
        }
    }
    free(slot);

    int first2 = 1;
    int base = -2;
    int max = 0;
    for(int i = 0; i < 100; i++){
        if(slots[i] != -2){
            if(first2){
                base = i;
                if(base > max)
                    max = base;
                first2 = 0;
            } else{
                if(base + slots[i] > max)
                    max = base + slots[i];
            }
        } else{
            first2 = 1;
        }
    }

    if(3695 - max < bytesNeeded){
        available = 0;
    }
    values.push_back(available);
    values.push_back(pageNumber);
    values.push_back(slot_location);
    values.push_back(max + 1);
    return values;
}

// Check for page with available slots, then check if there are adequate bytes
vector<int> RecordBasedFileManager::getPageSlotByte(FileHandle &fileHandle, unsigned numberOfSlots, unsigned bytesNeeded){
    unsigned lastPage = fileHandle.getNumberOfPages() - 1;
    // no pages yet, append a page
    if(lastPage == -1){
        void *newPage = malloc(PAGE_SIZE);
        memset(newPage, -2, PAGE_SIZE);
        fileHandle.appendPage(newPage);
        free(newPage);
        vector<int> r;
        r.push_back(0);
        r.push_back(0);
        r.push_back(0);
        return r;
    }
    vector<int> viable = this->getPageViability(fileHandle, numberOfSlots, bytesNeeded, lastPage);
    if(viable[0]){
        vector<int> r;
        r.push_back(viable[1]);
        r.push_back(viable[2]);
        r.push_back(viable[3]);
        return r;
    }
    else{
        for(int i = 0; i < fileHandle.getNumberOfPages(); i++){
            viable = this->getPageViability(fileHandle, numberOfSlots, bytesNeeded, i);
            if(viable[0]){
                vector<int> r;
                r.push_back(viable[1]);
                r.push_back(viable[2]);
                r.push_back(viable[3]);
                return r;
            }
        }
        // All pages are full, allocate a page and append it.
        void *newPage = malloc(PAGE_SIZE);
        memset(newPage, -2, PAGE_SIZE);
        fileHandle.appendPage(newPage);
        free(newPage);
        vector<int> r;
        r.push_back(fileHandle.getNumberOfPages() - 1);
        r.push_back(0);
        r.push_back(0);
        return r;
    }
}


RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	// read in page, write records to 
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
    
	void *page = malloc(PAGE_SIZE);
    	memset(page, 0, PAGE_SIZE);

	int *slots = (int*)calloc(numAttr+1, sizeof(int)); //arbitrary length
	
	unsigned offset = numNullBytes*sizeof(char);
	for (int i = 0; i < numAttr; i++)
	{
		string attrName = recordDescriptor[i].name;
		unsigned length = 4;	//I know there's a length in the recordDescriptor, 
					//but we always grab 4 bytes in the first grab of this program for varChar, int, and float
		//if null, indicate that in the record
		if (nullAttr[i] == 1)
		{
			//negative offset in slot table == null
            		slots[i+1] = -1;
		}
		//else, store the value
		else 
		{
			void* dataField;
			getField(data, offset, length, dataField);
			switch(recordDescriptor[i].type)
			{
				case 0 :	
					{
					}
						break;
			
				case 1 :	
					{
					}	
						break;
			
				case 2 :	
					{
                        			length = length + *(int*)dataField*sizeof(char); //leading int already factored in
					}
						break;
			}
			free (dataField);
			offset = offset + length;
			slots[i+1] = offset;
		}
	}
	
	vector<int> pageSlotByte;
	pageSlotByte =  getPageSlotByte(fileHandle, numAttr+1, offset);
	fileHandle.readPage(pageSlotByte[0], page);	
	putField(page, pageSlotByte[2], offset, data);

	slots[0] = pageSlotByte[2];

	putField(page, PAGE_SIZE-400-1+pageSlotByte[1], (numAttr+1)*sizeof(int), (void*)slots);

	

   

	rid.pageNum = pageSlotByte[0];	
	rid.slotNum = pageSlotByte[1];
    	fileHandle.writePage(rid.pageNum, page);
    	return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {

	// get page
    void *page = malloc(PAGE_SIZE);
    memset(page, 0, PAGE_SIZE);
    fileHandle.readPage(rid.pageNum, page);

    //offset to slots
    int startOfSlots = PAGE_SIZE-400-1;

    //number attributes
    int numAttr = recordDescriptor.size();

    //first and last slots relevant to our data.
    int ridSlot = startOfSlots + rid.slotNum*sizeof(int);

    //get offset to record start. Absolute offset from beginning of page
    void *recordStart = calloc(1, sizeof(int));
    getField(page, ridSlot, sizeof(int), recordStart);

    //get offset to record end. This is relative to record start
    int* slotArray = (int*)calloc(numAttr+1, sizeof(int));

    getField(page, ridSlot, (numAttr+1)*sizeof(int), (void*&)slotArray);

    int numNullBytes = ceil((float)numAttr/8.0);

    int totalSize = numNullBytes;
    for( int i = 1 ; i<= numAttr + 1; i++){
        //(void*)(((char*)data)+offset)
        //cout<<slotArray[i]<<endl;
        if( slotArray[i] > totalSize){
            totalSize = slotArray[i];
        }
    }

    _getField(page, *(int*)recordStart, totalSize, data);
    return 0;
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
