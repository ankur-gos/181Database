#include "pfm.h"
#include <fstream>
#include <string>
#include <iostream>

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}

PagedFileManager::PagedFileManager()
{
    // see page 325 in text
    // linked list heap file implementation? Keep name of header pages in known file (index) then 
    // on each page have next & prev page of the file so we can navigate
    // header pages have 2 links, 1 to full pages 1 to pages with free space
}


PagedFileManager::~PagedFileManager()
{
}

bool exists(const string &fileName){
    ifstream file("file_names");
    char aWord[50];
    while (file.good()) {
        file>>aWord;
        if (file.good() && strcmp(aWord, fileName.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

RC PagedFileManager::createFile(const string &fileName)
{
    // check to see if file already written
    if( exists(fileName) ){ return -1; }
    // format the string we want to write into the index
    char buffer [50];
    int n = sprintf (buffer, "%s\n", fileName.c_str());  

    // track the file in index
    FILE * name_fh = fopen( "file_names", "ab" ); 
    fputs(buffer, name_fh);
    fclose(name_fh);

    fclose(fopen(fileName.c_str(), "wb")); // create the file

    return 0;
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    //In progress --cj
    return -1;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    return -1;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    return -1;
}


FileHandle::FileHandle()
{
	readPageCounter = 0;
	writePageCounter = 0;
	appendPageCounter = 0;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
    return -1;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
	return -1;
}