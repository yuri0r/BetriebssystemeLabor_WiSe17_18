#include "fatManager.h"
#include <string.h>
#include <iostream>

FatBlockStruct *fbBuffer;
int bufferBlockIndex;

FatManager::FatManager() {
    fbBuffer = (FatBlockStruct *)malloc(BLOCK_SIZE);
    bufferBlockIndex = -1;
}

void FatManager::writeFat(BlockDevice* bd, int start, int destination) {
    int fatBlockCount = (start - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int startIndex = (start - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationBlock = (destination - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = (destination - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationIndex = destinationBlock * ADDRESS_COUNT_PER_FAT_BLOCK + destinationCount;

    if (fatBlockCount != bufferBlockIndex) {
        bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fbBuffer);
        bufferBlockIndex = fatBlockCount;
    }
    
    fbBuffer->destination[startIndex] = destinationIndex;

    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fbBuffer);
}

int FatManager::readFat(BlockDevice* bd, int position) {
    int fatBlockCount = position / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = position % ADDRESS_COUNT_PER_FAT_BLOCK;

    if (fatBlockCount != bufferBlockIndex) {
        bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fbBuffer);
        bufferBlockIndex = fatBlockCount;
    }
    return fbBuffer->destination[destinationCount];
}

int FatManager::expand(BlockDevice* bd, int currentLastFatAddress) {
    int nextFreeEntry = getFreeEntry(bd);

    if (nextFreeEntry == -1) {  // No free FAT block
        return -1;
    }

    if (currentLastFatAddress != -1) {
        writeFat(bd, currentLastFatAddress + FIRST_DATA_ADDRESS, nextFreeEntry + FIRST_DATA_ADDRESS);
    } 

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

    // No free FAT block
    return -1;
}

int FatManager::markEoF(BlockDevice* bd, int entry){ // Entry = DataAddress != FatAddress

    int fatBlock = entry / ADDRESS_COUNT_PER_FAT_BLOCK; //fat block which contains entrys
    int blockOffset = entry % ADDRESS_COUNT_PER_FAT_BLOCK; //which of the entries in a block

    if (fatBlock != bufferBlockIndex) {
        bd->read(FIRST_FAT_ADDRESS + fatBlock, (char *)fbBuffer);
        bufferBlockIndex = fatBlock;
    }

    std::cout << "EoF FatEntry: " << (fatBlock * ADDRESS_COUNT_PER_FAT_BLOCK) + blockOffset << std::endl; 
    int oldDestination = fbBuffer->destination[blockOffset];
    fbBuffer->destination[blockOffset] = -1; //checking for > 0 is fast  

    bd->write(FIRST_FAT_ADDRESS + fatBlock, (char *)fbBuffer);
    return oldDestination;
}

int FatManager::readAndClearEntry(BlockDevice* bd, int entry) {
    int fatBlockCount = entry / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = entry % ADDRESS_COUNT_PER_FAT_BLOCK;

    if (fatBlockCount != bufferBlockIndex) {
        bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fbBuffer);
        bufferBlockIndex = fatBlockCount;
    }

    int returnValue = fbBuffer->destination[destinationCount];
    fbBuffer->destination[destinationCount] = 0;
    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fbBuffer);
    
    return returnValue;
}
