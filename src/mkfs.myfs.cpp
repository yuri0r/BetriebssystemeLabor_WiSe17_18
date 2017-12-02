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
#include <iostream>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK_SIZE 512
#define MAX_FILE_SIZE (2 ^ 32) - 1
#define MAX_FILES 64

#define SUPER_BLOCK_ADRESS 0
#define SUPER_BLOCK_SIZE 1
#define SUPER_BLOCK_NAME 'myFS'

#define ROOT_ADRESS 1
#define ROOT_SIZE 1

#define INODES_ADRESS 2
#define INODES_SIZE MAX_FILES

#define FAT_ADRESS 65
#define FAT_SIZE (MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES)

#define DATA_ADRESS (FAT_ADRESS + FAT_SIZE)
#define DATA_SIZE //? in dont rly want to build a 128gb container file.... (@yuri)

struct stat *file_stat;

struct SuperBlock
{
    int name;
    int blockSize;
    int rootAdress;
    int inodesAdress;
    int fatAdress;
    int dataAdress;
};

struct Inode // Bytes: 256  + 3 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 32 + 32 = 344 @curvel
{
    char fileName[256] = {};    // act of pure rebelion! (also 255 is just ugly) @yuri
    char fileExtension[3] = {}; // file "type" (eg .txt)
    uint32_t fileSize;          // size of file
    uint32_t usedBlocksCount;   // how many 512B Blocks 
    uint8_t mode;               // rwx
    uint32_t atime;             // last access
    uint32_t mtime;             // last modification
    uint32_t ctime;             // last modification of status
    int firstFatEntry;          // pointer to fat
    char userID[32];       // id Of user
    char groupID[32];      // id of group
};

struct FatBlock
{
    int destination[16] = {};
} fatBlock;

BlockDevice *bd = new BlockDevice(BLOCK_SIZE);

void initSuperBlock()
{

    SuperBlock *sb = (SuperBlock *)malloc(BLOCK_SIZE * SUPER_BLOCK_SIZE);

    sb->name = SUPER_BLOCK_NAME;
    sb->blockSize = SUPER_BLOCK_SIZE;
    sb->rootAdress = ROOT_ADRESS;
    sb->inodesAdress = INODES_ADRESS;
    sb->fatAdress = FAT_ADRESS;
    sb->dataAdress = DATA_ADRESS;

    if (sizeof(sb) > BLOCK_SIZE)
    {
        printf("definition to large");
    }
    else
    {
        bd->write(SUPER_BLOCK_ADRESS, (char *)sb);
    }
}

void createInode(int inodeIndex,
                 char **fileName,
                 char **fileExtension,
                 uint32_t *fileSize,
                 uint32_t *usedBlocksCount,
                 uint8_t *mode,
                 uint32_t *atime,
                 uint32_t *mtime,
                 uint32_t *ctime,
                 int *firstFatEntry,
                 char **userID,
                 char **groupID)
{
    Inode *inode = (Inode *)malloc(BLOCK_SIZE);

    strcpy( inode->fileName, *fileName);
    strcpy( inode->fileExtension, *fileExtension);
    inode->fileSize = *fileSize;
    inode->usedBlocksCount = *usedBlocksCount;
    inode->mode = *mode;
    inode->atime = *atime;
    inode->mtime = *mtime;
    inode->ctime = *ctime;
    inode->firstFatEntry = *firstFatEntry;
    strcpy( inode->userID, *userID);
    strcpy( inode->groupID, *groupID);

    if (inodeIndex)
    {
        bd->write(inodeIndex + INODES_ADRESS, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not in Inode Space" ;
    }
}

void writeFat(int start, int destination)
{
    int fatBlockCount = (start - DATA_ADRESS) / 16;
    int destinationCount = (start - DATA_ADRESS) % 16;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FAT_ADRESS + fatBlockCount, (char *)fb);

    fb->destination[destinationCount] = destination;

    bd->write(FAT_ADRESS + fatBlockCount, (char *)fb);
}

int readFat(int position)
{
    int fatBlockCount = position / 16;
    int destinationCount = position % 16;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FAT_ADRESS + fatBlockCount, (char *)fb);

    return fb->destination[destinationCount];
}

void dataCreation(int argc, char *argv[])
{
    int addressCounter = 0;
    for (int i = 2; i < argc; i++)
    {
        std::streampos size;
        std::ifstream file(argv[i], std::ios::in | std::ios::binary | std::ios::ate); //openfile
        if (file.is_open())

        stat(argv[i], file_stat);
        
        {
            size = file.tellg();
            char *filebuffer = (char *)malloc(size); //save file localy
            file.seekg(0, std::ios::beg);
            file.read(filebuffer, size);
            file.close();
            for (int i = 0; i < size; i += BLOCK_SIZE)
            {
                char *filewriter = filebuffer + i;
                bd->write(DATA_ADRESS + addressCounter, filewriter);
                int j = i + BLOCK_SIZE;
                if (j < size)
                    writeFat(DATA_ADRESS + addressCounter, DATA_ADRESS + addressCounter + 1);
                addressCounter++;
            }

            std::cout << size << std::endl;
            free(filebuffer);
        }
    }
}

int main(int argc, char *argv[])
{

    char *containerPath;

    // TODO: Implement file system generation & copying of files here
    if (argc < 2)
    {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... file n>";
        return 1;
    }
    std::cout << "Argument count: " << argc << std::endl;

    containerPath = argv[1];
    bd->create(containerPath);

    for (int i = 0; i < argc; i++)
    {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        //bd->write(DATA_ADRESS + i, argv[i]);         //now done in DATA (@tristan)
    }

    // TODO create Superblock (done)
    initSuperBlock();

    //bd->write(0,buffer);

    // End of create Superblock

    // TODO create FAT

    // End of create FAT

    // TODO create INODES

    // End of create INODES

    // TODO create DATA
    dataCreation(argc, argv);

    // End of create DATA

    // TODO Calculate size of Binary file
    return 0;
}
