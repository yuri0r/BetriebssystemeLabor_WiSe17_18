#ifndef superBlock_h
#define superBlock_h

#include "myfs.h"
#include "blockdevice.h"
#include <stdlib.h>

struct SuperBlockStruct
{
    int fileSystemName;
    int blockSize;
    int maxFileSize;
    int maxFiles;
    int fatSize;
    int adressCounterPerFatBlock;
    int rootAdress;
    int inodesAdress;
    int firsFatAdress;
    int firstDataAdress;
};

class SuperBlock{
    public:
        SuperBlock();
        void initSuperBlock(BlockDevice* bd);
};

#endif