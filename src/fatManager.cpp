#include "fatManager.h"
#include <string.h>
#include <iostream>

FatBlockStruct *fbBuffer;
int bufferBlockIndex;
bool dmap[FAT_SIZE*ADDRESS_COUNT_PER_FAT_BLOCK] = {0};

FatManager::FatManager() {
    fbBuffer = (FatBlockStruct *)malloc(BLOCK_SIZE);
    bufferBlockIndex = -1;
}

void FatManager::writeFat(BlockDevice* bd, int index, int destinationIndex) {
    int blockIndex = index / ADDRESS_COUNT_PER_FAT_BLOCK;
    int inBlockIndex = index % ADDRESS_COUNT_PER_FAT_BLOCK;

    updateBuffer(bd, blockIndex);
    
    fbBuffer->destination[inBlockIndex] = destinationIndex;
    dmap[index] = 1;

    writeBuffer(bd, blockIndex);
}

int FatManager::readFat(BlockDevice* bd, int index) {
    int blockIndex = index / ADDRESS_COUNT_PER_FAT_BLOCK;
    int inBlockIndex = index % ADDRESS_COUNT_PER_FAT_BLOCK;

    updateBuffer(bd, blockIndex);

    return fbBuffer->destination[inBlockIndex];
}

int FatManager::expand(BlockDevice* bd, int currentLastIndex) {
    int nextIndex = getFreeIndex(bd);

    if (nextIndex == -1) {  // No free FAT block
        return -1;
    }

    if (currentLastIndex != -1) {
        writeFat(bd, currentLastIndex, nextIndex);
    } // else this is the first FAT block for this file

    return nextIndex;
}

int FatManager::getFreeIndex(BlockDevice* bd) {
    int destinationIndex = -1;

    for (long index = 0; index < (FAT_SIZE * ADDRESS_COUNT_PER_FAT_BLOCK); index++) {

        if (dmap[index] == 0) { // Entry is free
            markEoF(bd, index);
            return index;
        }
    }

    return -1; // No free FAT block
}

int FatManager::markEoF(BlockDevice* bd, int index){ // Entry = DataAddress != FatAddress
    int blockIndex = index / ADDRESS_COUNT_PER_FAT_BLOCK; 
    int inBlockIndex = index % ADDRESS_COUNT_PER_FAT_BLOCK; 

    updateBuffer(bd, blockIndex);
 
    int oldDestinationIndex = fbBuffer->destination[inBlockIndex];

    fbBuffer->destination[inBlockIndex] = -1;
    writeBuffer(bd, blockIndex);
    dmap[index] = 1;

    return oldDestinationIndex;
}

int FatManager::clearIndex(BlockDevice* bd, int index) {
    int blockIndex = index / ADDRESS_COUNT_PER_FAT_BLOCK;
    int inBlockIndex = index % ADDRESS_COUNT_PER_FAT_BLOCK;

    updateBuffer(bd, blockIndex);

    int oldDestinationIndex = fbBuffer->destination[inBlockIndex];

    fbBuffer->destination[inBlockIndex] = 0;
    writeBuffer(bd, blockIndex);
    dmap[index] = 0;
    
    return oldDestinationIndex;
}

int FatManager::updateBuffer(BlockDevice* bd, int blockIndex) {
    if (blockIndex != bufferBlockIndex) {
        bd->read(FIRST_FAT_ADDRESS + blockIndex, (char *)fbBuffer);
        bufferBlockIndex = blockIndex;
    }
}

void FatManager::writeBuffer(BlockDevice* bd, int blockIndex) {
    bd->write(FIRST_FAT_ADDRESS + blockIndex, (char *)fbBuffer);
}
