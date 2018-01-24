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
        int readFat(BlockDevice* bd, int index);
        void writeFat(BlockDevice* bd, int index, int destinationIndex);
        int markEoF(BlockDevice* bd, int index);
        int expand(BlockDevice* bd, int currentLastIndex);
        int clearIndex(BlockDevice* bd, int index);
    private:
        int getFreeIndex(BlockDevice* bd);
        int updateBuffer(BlockDevice* bd, int blockIndex);
        void writeBuffer(BlockDevice* bd, int blockIndex);
};

#endif