#include "inodeManager.h"
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
                               long mtime,
                               long ctime,
                               int firstFatEntry,
                               unsigned int userID,
                               unsigned int groupID)
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
    inode->mode = S_IFREG | 0444;

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
        std::cout << "ERROR not in Inode Space: " << inodeIndex << std::endl;
    }
}

InodeBlockStruct* InodeManager::getInode(BlockDevice *bd, const char *fileName)
{
    fileName++;
    InodeBlockStruct* node = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    for (int i = 0; i < MAX_FILES; i++ ){
        bd->read(INODES_ADDRESS + i, (char*)node);
        if (strcmp(node->fileName ,fileName)==0){
            return node;
        }
    }
    return NULL;
}

char* InodeManager::getFileName(BlockDevice *bd, int index){
    InodeBlockStruct *node = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    bd->read(INODES_ADDRESS + index, (char*)node);
    return node->fileName;
}
