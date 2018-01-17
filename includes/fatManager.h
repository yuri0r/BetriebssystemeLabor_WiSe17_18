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
        void markEoF(BlockDevice* bd, int entry);
        int readFat(BlockDevice* bd, int position);
        int expand(BlockDevice* bd, int currentLastFatAddress);
        int readAndClearEntry(BlockDevice* bd, int entry);
        int getFreeEntry(BlockDevice* bd);
    private:
        
};

#endif