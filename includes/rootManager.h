#ifndef rootBlockManager_h
#define rootBlockManager_h

#include "myfs.h"
#include "blockdevice.h"
#include "fsConfig.h"
#include <stdlib.h>
using namespace fsConfig;

struct RootBlockStruct
{
    // false = for inactive node
    // true  = active node
    bool inodesAddress[MAX_FILES] = {0};
};

class RootManager{
    public:
        RootManager();
        RootBlockStruct* rbStruct;
        void load(BlockDevice* bd);
        void setInode(BlockDevice* bd, int inodeIndex, bool active);
        RootBlockStruct getRootBlock(BlockDevice* bd);
        bool isValid(BlockDevice *bd, int index);
};

#endif