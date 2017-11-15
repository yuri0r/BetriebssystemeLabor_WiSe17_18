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

int main(int argc, char *argv[]) {

    // TODO: Implement file system generation & copying of files here
        std::cout << argc << std::endl ;

        for(int i=0;i<argc;i++){
            std::cout << argv[i] << std::endl ;
        }
    return 0;
}
