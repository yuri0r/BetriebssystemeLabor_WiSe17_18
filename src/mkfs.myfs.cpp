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

#define BLOCK_SIZE 512
#define MAX_FILE_SIZE 2 ^ 31
#define MAX_FILES 64

#define SUPER_BLOCK_ADRESS 0
#define SUPER_BLOCK_SIZE 1
#define SUPER_BLOCK_NAME 'myFS'

#define ROOT_ADRESS 1
#define ROOT_SIZE 1

#define INODES_ADRESS 2
#define INODES_SIZE MAX_FILES

#define FAT_ADRESS 65
#define FAT_SIZE MAX_FILE_SIZE / BLOCK_SIZE *MAX_FILES

#define DATA_ADRESS FAT_ADRESS + FAT_SIZE
#define DATA_SIZE //? in dont rly want to build a 128gb container file.... (@yuri)

struct SuperBlock
{
    int name;
    int blockSize;
    int rootAdress;
    int inodesAdress;
    int fatAdress;
    int dataAdress;
} superBlock;

void initSuperBlock(BlockDevice *bd)
{

    SuperBlock *sb = (SuperBlock *)malloc(BLOCK_SIZE * SUPER_BLOCK_SIZE);

    sb->name = SUPER_BLOCK_NAME;
    sb->blockSize = SUPER_BLOCK_SIZE;
    sb->rootAdress = ROOT_ADRESS;
    sb->inodesAdress = INODES_ADRESS;
    sb->fatAdress = FAT_ADRESS;
    sb->dataAdress = DATA_ADRESS;

    if (sizeof(sb) > 512)
    {
        printf("definition to large");
        exit;
    }
    bd->write(SUPER_BLOCK_ADRESS, (char *)sb);

    /*test it worked...(@yuri)
    SuperBlock *nsb = (SuperBlock *)malloc(BLOCK_SIZE * SUPER_BLOCK_SIZE);

    bd->read(SUPER_BLOCK_ADRESS, (char *)nsb);

    std::cout << "nsb \t orig sb \n" <<
           nsb->name << '\t' << sb->name << '\n' <<
           nsb->blockSize << '\t' << sb->blockSize << '\n' <<
           nsb->rootAdress << '\t' << sb->rootAdress << '\n' <<
           nsb->inodesAdress << '\t' << sb->inodesAdress << '\n' <<
           nsb->fatAdress << '\t' << sb->fatAdress << '\n' <<
           nsb->dataAdress << '\t' << sb->dataAdress << '\n';

    free(nsb);
    free(sb);
    */
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
    BlockDevice *bd = new BlockDevice(BLOCK_SIZE);
    bd->create(containerPath);

    for (unsigned int i = 2; i < argc; i++)
    {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        //bd->write(DATA_ADRESS + i, argv[i]);         //argv[] can not exceed one block in size
    }

    // TODO create Superblock (done)
    initSuperBlock(bd);
    
    //bd->write(0,buffer);

    // End of create Superblock

    // TODO create FAT

    // End of create FAT

    // TODO create INODES

    // End of create INODES

    // TODO create DATA

    // End of create DATA

    // TODO Calculate size of Binary file

    return 0;
}
