#include "inodeManager.h"
#include "rootManager.h"
#include "fsConfig.h"
#include "blockdevice.h"
#include <string.h>
#include <iostream>
using namespace fsConfig;

InodeManager::InodeManager() {}

void InodeManager::createInode(BlockDevice *bd, int inodeIndex,
                               char *fileName,
                               long fileSize,
                               long usedBlocksCount,
                               long atime,
                               unsigned long atime_nsec,
                               long mtime,
                               unsigned long mtime_nsec,
                               long ctime,
                               unsigned long ctime_nsec,
                               int firstFatEntry,
                               unsigned int userID,
                               unsigned int groupID,
                               unsigned int mode)
{
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    strcpy(inode->fileName, fileName);
    inode->fileSize = fileSize;
    inode->usedBlocksCount = usedBlocksCount;
    inode->atime = atime;
    inode->atime_nsec = atime_nsec;
    inode->mtime = mtime;
    inode->mtime_nsec = mtime_nsec;
    inode->ctime = ctime;
    inode->ctime_nsec = ctime_nsec;
    inode->firstFatEntry = firstFatEntry;
    inode->userID = userID;
    inode->groupID = groupID;
    inode->mode = mode;
    inode->index = inodeIndex;

    std::cout << "File: " << fileName << std::endl
              << "Size: " << fileSize << "Byte" << std::endl
              << "used Blocks: " << usedBlocksCount << std::endl
              << "firstFatEntry: " << firstFatEntry << std::endl
              << std::endl;

    if (inodeIndex >= 0 && inodeIndex < (FIRST_FAT_ADDRESS - INODES_ADDRESS))
    {
        bd->write(inodeIndex + INODES_ADDRESS, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not inn Inode Space: " << inodeIndex << std::endl;
    }
}

void InodeManager::createInode(BlockDevice *bd, int inodeIndex,
                               char *fileName,
                               long fileSize,
                               long usedBlocksCount,
                               long atime,
                               long mtime,
                               long ctime,
                               int firstFatEntry,
                               unsigned int userID,
                               unsigned int groupID,
                               unsigned int mode)
{
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    strcpy(inode->fileName, fileName);
    inode->fileSize = fileSize;
    inode->usedBlocksCount = usedBlocksCount;
    inode->atime = atime;
    inode->mtime = mtime;
    inode->ctime = ctime;
    inode->firstFatEntry = firstFatEntry;
    inode->userID = userID;
    inode->groupID = groupID;
    inode->mode = mode;
    inode->index = inodeIndex;

    std::cout << "File: " << fileName << std::endl
              << "Size: " << fileSize << "Byte" << std::endl
              << "used Blocks: " << usedBlocksCount << std::endl
              << "firstFatEntry: " << firstFatEntry << std::endl
              << std::endl;

    if (inodeIndex >= 0 && inodeIndex < (FIRST_FAT_ADDRESS - INODES_ADDRESS))
    {
        bd->write(inodeIndex + INODES_ADDRESS, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not inn Inode Space: " << inodeIndex << std::endl;
    }
}

InodeBlockStruct* InodeManager::getInode(BlockDevice *bd, RootManager *rmgr, const char *fileName) {
    fileName++;
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    for (int i = 0; i < MAX_FILES; i++) {
        if (rmgr->isValid(bd, i)) {
            inode = getInodeByIndex(bd, i); 
            if (strcmp(inode->fileName ,fileName) == 0){
                return inode;
            }
        }
    }

    free(inode);
    return NULL;
}

InodeBlockStruct* InodeManager::getInodeByIndex(BlockDevice *bd, int index)
{
    InodeBlockStruct* node = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    bd->read(index + INODES_ADDRESS, (char *)node);
    return node;
}

char* InodeManager::getFileName(BlockDevice *bd, int index){
    InodeBlockStruct *node = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    bd->read(INODES_ADDRESS + index, (char*)node);
    return node->fileName;
}

void InodeManager::updateInode(BlockDevice *bd, InodeBlockStruct *inode) {
    bd->write(INODES_ADDRESS + inode->index, (char*)inode);
}

InodeBlockStruct* InodeManager::clearValidInode(BlockDevice *bd, RootManager *rmgr, const char *fileName) {
    fileName++;
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    for (int i = 0; i < MAX_FILES; i++) {
        if (rmgr->isValid(bd, i)) {
            inode = getInodeByIndex(bd, i); 
            if (strcmp(inode->fileName ,fileName) == 0){
                rmgr->setInode(bd, i, false);
                return inode;
            }
        }
    }

    free(inode);
    return NULL;
}