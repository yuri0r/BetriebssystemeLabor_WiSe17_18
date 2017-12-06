//
//  mk.myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"
#include "superBlock.h"
#include "fsConfig.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace fsConfig;

// ***********************start structs******************************


struct RootBlock
{
    // false = for inactive node
    // true  = active node
    bool inodesAddress[MAX_FILES] = {0};
};

struct InodeBlock // Bytes: 256  + 3 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 32 + 32 = 344 @curvel this is outdated @yuri
{
    char fileName[256];       // act of pure rebelion! (also 255 is just ugly) @yuri
    long fileSize;            // size of file in bytes
    long usedBlocksCount;     // how many 512B Blocks
    unsigned int mode = 0444; // rwx
    long atime;               // last access
    long mtime;               // last modification
    long ctime;               // last modification of status
    int firstFatEntry;        // pointer to fat
    unsigned int userID;      // id Of user
    unsigned int groupID;     // id of group
};

struct FatBlock
{
    int destination[ADDRESS_COUNT_PER_FAT_BLOCK] = {};
};

// ***************end structs**************************************

BlockDevice *bd = new BlockDevice(BLOCK_SIZE);
SuperBlock *sb = new SuperBlock();

void setInodeInRoot(int inodeIndex, bool active)
{
    RootBlock *rb = (RootBlock *)malloc(BLOCK_SIZE);

    bd->read(ROOT_ADDRESS, (char *)rb);

    rb->inodesAddress[inodeIndex] = active;

    bd->write(ROOT_ADDRESS, (char *)rb);

    free(rb);
}

void createInode(int inodeIndex,
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
    InodeBlock *inode = (InodeBlock *)malloc(BLOCK_SIZE);

    strcpy(inode->fileName, fileName);
    inode->fileSize = fileSize;
    inode->usedBlocksCount = usedBlocksCount;
    inode->atime = atime;
    inode->mtime = mtime;
    inode->ctime = ctime;
    inode->firstFatEntry = firstFatEntry;
    inode->userID = userID;
    inode->groupID = groupID;

    std::cout   << "File: " << fileName << std::endl
                        << "Size: " << fileSize << "Byte" << std::endl
                        << "used Blocks: " << usedBlocksCount << std::endl
                        << "firstFatEntry: " << firstFatEntry << std::endl << std::endl;

    if (inodeIndex >= 0 && inodeIndex < (FIRST_FAT_ADDRESS - INODES_ADDRESS))
    {
        bd->write(inodeIndex + INODES_ADDRESS, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not in Inode Space: " << inodeIndex << std::endl;
    }
}

void writeFat(int start, int destination)
{
    int fatBlockCount = (start - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int startIndex = (start - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationBlock = (destination - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = (destination - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationIndex = destinationBlock * ADDRESS_COUNT_PER_FAT_BLOCK + destinationCount;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    fb->destination[startIndex] = destinationIndex;

    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);
}

int readFat(int position)
{
    int fatBlockCount = position / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = position % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    return fb->destination[destinationCount];
}

char *formatFileName(char *input)
{
    int count = 0;
    while (*input != '\0')
    {
        input++;
        count++;
    }

    while (*input != '/' && count > 0)
    {
        input--;
        count--;
    }
    if (*input == '/')
    {
        return input + 1;
    }
    else
    {
        return input;
    }
}

bool checkDuplicate (char* fileName, int argcNum, int argc, char *argv[]){
    char* testName;
    for(int i = (argcNum-1); i > 1; i--){
        testName = formatFileName(argv[i]);
        if(strcmp(testName,fileName)==0){
             return true;
        }
    }
    return false;
}

void dataCreation(int argc, char *argv[])
{
    int addressCounter = 0;
    int firstEntry;
    int blocksUsed;

    for (int i = 2; i < argc; i++)
    {
        char *fileName = argv[i];
        fileName = formatFileName(fileName);
        if (!checkDuplicate(fileName, i, argc, argv)) {
            std::streampos size;
            std::ifstream file(argv[i], std::ios::in | std::ios::binary | std::ios::ate); //openfile
            if (file.is_open())
            {
                size = file.tellg();
                char *filebuffer = (char *)malloc(size); //save file localy
                file.seekg(0, std::ios::beg);
                file.read(filebuffer, size);
                file.close();
                firstEntry = addressCounter;
                blocksUsed = 1;
                for (int i = 0; i < size; i += BLOCK_SIZE)
                {
                    char *filewriter = filebuffer + i;
                    bd->write(FIRST_DATA_ADDRESS + addressCounter, filewriter);
                    int j = i + BLOCK_SIZE;
                    if (j < size)
                    {
                        writeFat(FIRST_DATA_ADDRESS + addressCounter, FIRST_DATA_ADDRESS + addressCounter + 1);
                        blocksUsed++;
                    }
                    addressCounter++;
                }
                //set inode and root entries
                struct stat fs;
                stat(fileName, &fs);

                setInodeInRoot(i - 2, true); //marks inode as valid in root
                createInode(i - 2,
                            fileName,
                            fs.st_size,
                            blocksUsed,
                            fs.st_atime,
                            fs.st_mtime,
                            fs.st_ctime,
                            firstEntry,
                            fs.st_uid,
                            fs.st_gid);
                
                free(filebuffer);
            }
        } else {
            std::cout << "File: " << argv[i] << "\nname allready in use! \n" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... file n>";
        return 1;
    }

    std::cout << "ContainerFile: " << argv[1] << std::endl;

    bd->create(argv[1]); // argv[1] = containerPath

    sb->initSuperBlock(bd);

    dataCreation(argc, argv);

    return 0;
}
