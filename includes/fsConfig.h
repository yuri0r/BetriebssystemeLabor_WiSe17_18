#ifndef fsConfig_h
#define fsConfig_h

namespace fsConfig{
    const int FILE_SYSTEM_NAME = 'myFS';

    const int BLOCK_SIZE = 512;
    const int MAX_FILE_SIZE = (2 ^ 32) - 1;
    const int MAX_FILES = 64;

    const int ROOT_ADDRESS = 1;
    const int INODES_ADDRESS = 2;
    const int FIRST_FAT_ADDRESS = (MAX_FILES + INODES_ADDRESS);

    const int FAT_SIZE = (MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES);
    const int FIRST_DATA_ADDRESS = (FIRST_FAT_ADDRESS + FAT_SIZE);
    const int ADDRESS_COUNT_PER_FAT_BLOCK = (BLOCK_SIZE / 4);
}

#endif