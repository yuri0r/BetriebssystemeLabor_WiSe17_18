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
#define MAX_FILE_SIZE 2^31
#define MAX_FILES 64

#define SUPER_BLOCK_ADRESS 0
#define SUPER_BLOCK_SIZE 1
#define SUPER_BLOCK_NAME "myFS"

#define SUPER_BLOCK_NAME_POSITION 0
#define SUPER_BLOCK_BLOCK_SIZE_POSITION 32
#define SUPER_BLOCK_MAX_FILE_SIZE_POSITION 64
#define SUPER_BLOCK_MAX_FILES_POSITION 96
#define SUPER_BLOCK_ROOT_ADRESS_POSITION 128
#define SUPER_BLOCK_INODES_POSITION 160
#define SUPER_BLOCK_FAT_ADRESS_POSITION 192
#define SUPER_BLOCK_DATA_ADRESS_POSITION 224

#define ROOT_ADRESS 1
#define ROOT_SIZE 1

#define INODES_ADRESS 2
#define INODES_SIZE MAX_FILES

#define FAT_ADRESS  65
#define FAT_SIZE MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES

#define DATA_ADRESS FAT_ADRESS + FAT_SIZE
#define DATA_SIZE //? in dont rly want to build a 128gb container file.... (@yuri)

int main(int argc, char *argv[]) {

    char* containerPath;
    
    // TODO: Implement file system generation & copying of files here
    if (argc < 2) {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... file n>";
        return 1;
    }
    std::cout << "Argument count: " << argc << std::endl;
    
    containerPath = argv[1];
    BlockDevice *bd  = new BlockDevice(BLOCK_SIZE);
    bd->create(containerPath);

    for(unsigned int i=2; i<argc; i++){
        std::cout << "Argument " << i << ": "  <<  argv[i] << std::endl;
        bd->write(DATA_ADRESS + i, argv[i]);         //argv[] can not exceed one block in size
    }

    // TODO create Superblock
    char null = NULL;
    char* buffer = (char*)malloc(BLOCK_SIZE);
    const char* blockSizePosition = (std::to_string(SUPER_BLOCK_BLOCK_SIZE_POSITION)).c_str();
    unsigned int offset = 0;

    //fill with super block information
    for (unsigned int i = 0; i < sizeof(SUPER_BLOCK_NAME); i++ ){
        offset = SUPER_BLOCK_NAME_POSITION;
        buffer[i + offset] = SUPER_BLOCK_NAME[i + offset];
    }

    for (unsigned int i = 0; i < sizeof(blockSizePosition); i++ ){
        buffer[i + SUPER_BLOCK_BLOCK_SIZE_POSITION] = blockSizePosition[i + SUPER_BLOCK_BLOCK_SIZE_POSITION];
        std::cout << buffer[i+32] <<"yes pls";
    }

    for(unsigned int i = 0; i < BLOCK_SIZE * SUPER_BLOCK_SIZE; i++) {    
        //buffer[i] = null;
    }
    
    std::cout << buffer << std::endl << blockSizePosition;
    bd->write(0,buffer);

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
