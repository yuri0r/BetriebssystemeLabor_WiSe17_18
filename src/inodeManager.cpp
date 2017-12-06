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

InodeBlockStruct InodeManager::getInode(BlockDevice *bd, char *fileName)
{
char *buff;
InodeBlockStruct *node;

    for (int i = INODES_ADDRESS; i < MAX_FILES; i++ ){
        node = (InodeBlockStruct*)bd->read(INODES_ADDRESS,buff);
        if (strcmp(node->fileName ,fileName) == true){
            return *node;
        }
    }
}
