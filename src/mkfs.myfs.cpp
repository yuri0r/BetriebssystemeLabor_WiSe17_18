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
        std::cout << argc << std::endl ;

        for(int i=0;i<argc;i++){
            std::cout << argv[i] << std::endl ;
        }

        if (argc < 2) {
            std::cout << "usuage ./mkfs.myfs <files to be added>" ;
            return 1;
        }

        //TODO Calculate size of Binary file
        
        
        BlockDevice bd;
        bd.create(CONTAINER);
        bd.open(CONTAINER);
        char example = 'c';

        bd.write(0,&example);

    return 0;
}
