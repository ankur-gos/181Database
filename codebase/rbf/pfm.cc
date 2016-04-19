#include "pfm.h"
#include <fstream>
#include <string>
#include <cstring>	//needed to add to get strcmp working
#include <iostream>
#include <unistd.h>
#include <stdio.h>

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

void deleteEntry(const string &fileName){
    // copy all lines not matching the fileName into a temp file then rename temp->file_name
    FILE * temp_fh = fopen( "temp", "wb" ); 
    ifstream file("file_names");
    char aWord[50];
    while (file.good()) {
        file>>aWord;
        if (file.good() && strcmp(aWord, fileName.c_str()) != 0) {
            fputs(aWord, temp_fh);
        }
    }
    remove("file_names");
    rename("temp", "file_names");
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
    //if( !exists(fileName) ){ return -1; }
    deleteEntry(fileName);
    remove(fileName.c_str());
    return 0;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    if(!exists(fileName)){ return -1; }
    if(fileHandle.file != NULL){
        // fileHandle already associated with a file
        return -1;
    }
    fileHandle.file = fopen(fileName.c_str(), "rb+");
    if(fileHandle.file == NULL){
        return -1;
    }
    return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if(fileHandle.file == NULL){
        return -1;
    }
    int err = fflush(fileHandle.file);
    if(err != 0){
        return err;
    }
    fclose(fileHandle.file);
    fileHandle.file = NULL;
    return 0;
}


FileHandle::FileHandle()
{
	readPageCounter = 0;
	writePageCounter = 0;
	appendPageCounter = 0;
    file = NULL;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    if(this->file == NULL){
        return -1;
    }

    if(pageNum > this->getNumberOfPages()){
        return -1;
    }
    // Seek to the right page, read the page, seek back to beginning.
    fseek(this->file, pageNum * PAGE_SIZE, SEEK_SET);
    fread(data, 1, PAGE_SIZE, this->file);
    fseek(this->file, 0, SEEK_SET);

    readPageCounter = readPageCounter + 1;
    return 0;
}

RC FileHandle::writeAndFlush(long offset, int location, const void *data){
    if(this->file == NULL){
        return -1;
    }
    fseek(this->file, offset, location);
    fwrite(data, 1, PAGE_SIZE, this->file);
    fseek(this->file, 0, SEEK_SET);
    int err = fflush(file);
    if(err != 0){
        return err;
    }
    return 0;
}

RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    if(pageNum > this->getNumberOfPages()){
        return -1;
    }
    RC err = this->writeAndFlush(pageNum * PAGE_SIZE, SEEK_SET, data);
    if(err != 0){
        return err;
    }
    writePageCounter = writePageCounter + 1;
    return 0;
}


RC FileHandle::appendPage(const void *data)
{
    RC err = this->writeAndFlush(0, SEEK_END, data);
    if(err != 0){
        return err;
    }
    appendPageCounter = appendPageCounter + 1;
    return 0;
}


unsigned FileHandle::getNumberOfPages()
{
    // Get the file size in bytes, divide it by the page size to get the number of pages
    FILE *f = this->file;
    if(file == NULL){
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size / PAGE_SIZE;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = readPageCounter;
    writePageCount = writePageCounter;
    appendPageCount = appendPageCounter;
	return 0;
}
