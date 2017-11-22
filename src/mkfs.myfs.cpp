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
#define CONTAINER "container.bin"

int main(int argc, char *argv[]) {

    char* containerPath;
    const int maxFileSize= 2^31;  //2^31 = 2 Gigybite MAX Size allowed = 30 Gigybite
    const int sizeFAT = maxFileSize / 512 * 64;   //Size of the FAT block in blocks
    // TODO: Implement file system generation & copying of files here

    if (argc < 2) {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... filen>";
        return 1;
    }
    std::cout << "Argument count: " << argc << std::endl;
    
    containerPath = argv[1];
    BlockDevice *bd  = new BlockDevice(512);
    bd->create(containerPath);

    for(int i=0;i<argc;i++){
        std::cout << "Argument " << i << ": "  <<  argv[i] << std::endl;
        bd->write(i+66+sizeFAT,argv[i]);         //argv[] can not exceed one block in size
    }

    // TODO create Superblock

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
