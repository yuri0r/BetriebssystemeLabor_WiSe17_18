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

    // TODO: Implement file system generation & copying of files here

    if (argc < 2) {
        std::cout << "usuage ./mkfs.myfs <files to be added>";
        return 1;
    }

    std::cout << "Argument count: " << argc << std::endl;

    BlockDevice bd;
    bd.create(CONTAINER);
    bd.open(CONTAINER);

    for(int i=0;i<argc;i++){
        std::cout << "Argument " << i << ": "  <<  argv[i] << std::endl;
        bd.write(i,argv[i]);
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
