#ifndef containerReader_h
#define containerReader_h

#include "myfs.h"
#include "blockdevice.h"
#include <stdlib.h>
          
class ContainerReader {
    public:
        ContainerReader();
        char* reader (BlockDevice* bd, int block, int byteStart, int byteEnd);
        char* reader (BlockDevice* bd, int byteStart, int byteEnd);
};

#endif