#include "rootManager.h"
#include <string.h>
#include <iostream>

RootManager::RootManager() {}

void RootManager::setInode(BlockDevice* bd, int inodeIndex, bool active) {
    RootBlockStruct *rb = (RootBlockStruct *)malloc(BLOCK_SIZE);

    bd->read(ROOT_ADDRESS, (char *)rb);

    rb->inodesAddress[inodeIndex] = active;

    bd->write(ROOT_ADDRESS, (char *)rb);

    free(rb);
}