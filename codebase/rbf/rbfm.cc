#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{// to be implemented in project 1
    ordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    // to be implemented in project 1
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    // to be implemented in project 1
    return -1;
}
