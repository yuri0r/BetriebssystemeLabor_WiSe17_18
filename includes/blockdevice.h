//
//  blockdevice.h
//  myfs
//
//  Created by Oliver Waldhorst on 09.10.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef blockdevice_h
#define blockdevice_h

#include <stdio.h>
#include <cstdint>

#define BD_BLOCK_SIZE 512

class BlockDevice {
private:
    uint32_t blockSize;
    int contFile;
    uint32_t size;

public:
    BlockDevice(u_int32_t blockSize = 512);
    void resize(u_int32_t blockSize);
    int open(const char* path);
    int create(const char* path);    
    int close();
    int read(u_int32_t blockNo, char *buffer);
    int write(u_int32_t blockNo, char *buffer);
    uint32_t getSize() {return this->size;}
};

#endif /* blockdevice_h */
