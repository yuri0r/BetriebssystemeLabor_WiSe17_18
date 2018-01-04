#ifndef fatManager_h
#define fatManager_h

#include "myfs.h"
#include "blockdevice.h"
#include "fsConfig.h"
#include <stdlib.h>
using namespace fsConfig;

struct FatBlockStruct
{
    int destination[ADDRESS_COUNT_PER_FAT_BLOCK] = {};
};

class FatManager{
    public:
        FatManager();
        void writeFat(BlockDevice* bd, int start, int destination);
        int readFat(BlockDevice* bd, int position);
        
};

#endif