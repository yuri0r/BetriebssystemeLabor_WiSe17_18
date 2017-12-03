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
#define FILE_SYSTEM_NAME 'myFS'

#define ROOT_ADRESS 1
#define ROOT_SIZE 1

#define INODES_ADDRESS_OFFSET 2

#define FAT_ADDRESS_OFFSET 65
#define FAT_SIZE (MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES)

#define DATA_ADDRESS_OFFSET (FAT_ADDRESS_OFFSET + FAT_SIZE)

#define addressCountPerBlock 128

struct SuperBlock
{
    int name;
    int blockSize;
    int rootAdress;
    int inodesAdress;
    int fatAdress;
    int dataAdress;
};

struct RootBlock 
{
    // false for invalid node
    // true  valid node
    bool inodesAddress[MAX_FILES]={0};
};

struct Inode // Bytes: 256  + 3 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 32 + 32 = 344 @curvel this is outdated @yuri
{
    char fileName[256];    // act of pure rebelion! (also 255 is just ugly) @yuri
    long fileSize;              // size of file in bytes
    long usedBlocksCount;       // how many 512B Blocks
    unsigned int mode;          // rwx
    long atime;                 // last access
    long mtime;                 // last modification
    long ctime;                 // last modification of status
    int firstFatEntry;          // pointer to fat
    unsigned int userID;        // id Of user
    unsigned int groupID;       // id of group
};

struct FatBlock
{
    int destination[addressCountPerBlock] = {};
} fatBlock;

BlockDevice *bd = new BlockDevice(BLOCK_SIZE);

void initSuperBlock()
{
    SuperBlock *sb = (SuperBlock *)malloc(BLOCK_SIZE);

    sb->name = FILE_SYSTEM_NAME;
    sb->blockSize = BLOCK_SIZE;
    sb->rootAdress = ROOT_ADRESS;
    sb->inodesAdress = INODES_ADDRESS_OFFSET;
    sb->fatAdress = FAT_ADDRESS_OFFSET;
    sb->dataAdress = DATA_ADDRESS_OFFSET;

    if (sizeof(sb) > BLOCK_SIZE)
    {
        printf("definition to large");
    }
    else
    {
        bd->write(SUPER_BLOCK_ADRESS, (char *)sb);
    }
}

void setInodeInRoot(int inodeIndex, bool active)
{
  RootBlock *rb = (RootBlock *)malloc(BLOCK_SIZE);
    
  bd->read(ROOT_ADRESS, (char *)rb);
  
  rb->inodesAddress[inodeIndex] = active;
  
  bd->write(ROOT_ADRESS, (char *)rb);

  free(rb);
}

void createInode(int inodeIndex,
                 char *fileName,
                 long fileSize,
                 long usedBlocksCount,
                 unsigned int mode,
                 long atime,
                 long mtime,
                 long ctime,
                 int firstFatEntry,
                 unsigned int userID,
                 unsigned int groupID)
{
    Inode *inode = (Inode *)malloc(BLOCK_SIZE);

    strcpy(inode->fileName, fileName);
    inode->fileSize = fileSize;
    inode->usedBlocksCount = usedBlocksCount;
    inode->mode = mode;
    inode->atime = atime;
    inode->mtime = mtime;
    inode->ctime = ctime;
    inode->firstFatEntry = firstFatEntry;
    inode->userID = userID;
    inode->groupID = groupID;

    if (inodeIndex >= 0 && inodeIndex < (FAT_ADDRESS_OFFSET - INODES_ADDRESS_OFFSET))
    {
        bd->write(inodeIndex + INODES_ADDRESS_OFFSET, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not in Inode Space: " << inodeIndex << std::endl;
    }
}

void writeFat(int start, int destination)
{
    int fatBlockCount = (start - DATA_ADDRESS_OFFSET) / addressCountPerBlock;
    int destinationCount = (start - DATA_ADDRESS_OFFSET) % addressCountPerBlock;

    int destinationIndex = (destination - DATA_ADDRESS_OFFSET) / addressCountPerBlock + (destination - DATA_ADDRESS_OFFSET) % addressCountPerBlock;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FAT_ADDRESS_OFFSET + fatBlockCount, (char *)fb);

    fb->destination[destinationCount] = destinationIndex;

    bd->write(FAT_ADDRESS_OFFSET + fatBlockCount, (char *)fb);
}

int readFat(int position)
{
    int fatBlockCount = position / addressCountPerBlock;
    int destinationCount = position % addressCountPerBlock;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FAT_ADDRESS_OFFSET + fatBlockCount, (char *)fb);

    return fb->destination[destinationCount];
}

void dataCreation(int argc, char *argv[])
{
    int addressCounter = 0;
    int firstEntry;
    int blocksUsed;
    int filecount = 0;
    
    for (int i = 2; i < argc; i++)
    {
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
                bd->write(DATA_ADDRESS_OFFSET + addressCounter, filewriter);
                int j = i + BLOCK_SIZE;
                addressCounter++;
                if (j < size){
                    writeFat(DATA_ADDRESS_OFFSET + addressCounter, DATA_ADDRESS_OFFSET + addressCounter + 1);
                    blocksUsed ++;
                }
                
            }
            //set inode and root entries
            struct stat fs;
            stat(argv[i], &fs);
            setInodeInRoot( i - 2 , true);
            createInode(i-2 ,
                        argv[i],
                        fs.st_size,
                        blocksUsed,
                        fs.st_mode,
                        fs.st_atime,
                        fs.st_mtime, 
                        fs.st_ctime,
                        firstEntry,
                        fs.st_uid,
                        fs.st_gid);

            std::cout << "File " << i - 1 << ": \"" << argv[i] << "\", Size: " << size << "Byte"<< std::endl;
            free(filebuffer);
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

    initSuperBlock();

    dataCreation(argc, argv);

    return 0;
}
