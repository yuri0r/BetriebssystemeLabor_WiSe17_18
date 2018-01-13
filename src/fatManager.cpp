#include "fatManager.h"
#include <string.h>
#include <iostream>

FatManager::FatManager() {}

void FatManager::writeFat(BlockDevice* bd, int start, int destination) {
    int fatBlockCount = (start - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int startIndex = (start - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationBlock = (destination - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = (destination - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationIndex = destinationBlock * ADDRESS_COUNT_PER_FAT_BLOCK + destinationCount;

    FatBlockStruct *fb = (FatBlockStruct *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    fb->destination[startIndex] = destinationIndex;

    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    free(fb);
}

int FatManager::readFat(BlockDevice* bd, int position) {
    int fatBlockCount = position / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = position % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlockStruct *fb = (FatBlockStruct *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);
    int returnValue = fb->destination[destinationCount];
    free(fb);
    return returnValue;
}