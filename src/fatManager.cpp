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

int FatManager::expand(BlockDevice* bd, int currentLastFatAddress) {
    int fatBlockCount = currentLastFatAddress / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = currentLastFatAddress % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlockStruct *fb = (FatBlockStruct *)malloc(BLOCK_SIZE);
    int nextFreeEntry = getFreeEntry(bd);

    if (nextFreeEntry == -1) {
        // No fat block is empty
        return -1;
    }

    if (currentLastFatAddress != -1) {
        writeFat(bd, currentLastFatAddress + FIRST_DATA_ADDRESS, nextFreeEntry + FIRST_DATA_ADDRESS);
    } 

    free(fb);
    return nextFreeEntry;
}

int FatManager::getFreeEntry(BlockDevice* bd) {
    for (int position = 0; position < (FAT_SIZE * ADDRESS_COUNT_PER_FAT_BLOCK); position++) {
        int nextEntry = readFat(bd, position);

        if (nextEntry == 0) {
            markEoF(bd, position);
            return position;
        }
    }

    // No fat block is empty
    return -1;
}

void FatManager::markEoF(BlockDevice* bd, int entry){ // Entry = DataAddress != FatAddress

    int fatBlock = entry / ADDRESS_COUNT_PER_FAT_BLOCK; //fat block which contains entrys
    int blockOffset = entry % ADDRESS_COUNT_PER_FAT_BLOCK; //which of the entries in a block

    FatBlockStruct *fb = (FatBlockStruct *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlock, (char *)fb);

    std::cout << "EoF FatEntry: " << (fatBlock * ADDRESS_COUNT_PER_FAT_BLOCK) + blockOffset << std::endl; 
    fb->destination[blockOffset] = -1; //checking for > 0 is fast  

    bd->write(FIRST_FAT_ADDRESS + fatBlock, (char *)fb);
    free(fb);
}

int FatManager::readAndClearEntry(BlockDevice* bd, int entry) {
    int fatBlockCount = entry / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = entry % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlockStruct *fb = (FatBlockStruct *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);
    int returnValue = fb->destination[destinationCount];
    fb->destination[destinationCount] = 0;
    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);
    free(fb);
    
    return returnValue;
}
