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

    std::cout << argc << std::endl;
        BlockDevice bd;
        bd.create(CONTAINER);
        bd.resize(10);
        bd.open(CONTAINER);
        

    for(int i=0;i<argc;i++){
        std::cout << argv[i] << std::endl;
        bd.write(i,argv[i]);
    }

        //TODO Calculate size of Binary file
     bd.close();
    return 0;
}
