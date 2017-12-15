#include "containerReader.h"
#include "fsConfig.h"
#include <iostream>
#include <string.h>
using namespace fsConfig;

ContainerReader::ContainerReader () {}

//reads block, returns requested bytes (range 0 - 511), buffer saves last read block 
char *ContainerReader::reader (BlockDevice* bd, int block, int byteStart, int byteEnd){
    char result[(byteEnd-byteStart)];
    char buffer[BLOCK_SIZE];
    int bufferBlockNr = -1;                     //unused blocknr so reader always reads on first call

    if(block!=bufferBlockNr){
        bd->read(block, buffer);
        bufferBlockNr = block;
    }

    for(int i = 0; i <= (byteEnd-byteStart); i++){
        result[i] = buffer[i+byteStart];
    }

    return result;
}

//for reading bytes given relative to the start of the container 
char *ContainerReader::reader (BlockDevice* bd, int byteStart, int byteEnd){
    return ContainerReader::reader(bd, byteStart/BLOCK_SIZE, byteStart%BLOCK_SIZE, byteEnd%BLOCK_SIZE);
}