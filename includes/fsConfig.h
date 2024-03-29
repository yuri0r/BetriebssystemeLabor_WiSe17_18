#ifndef fsConfig_h
#define fsConfig_h

namespace fsConfig{
    const int FILE_SYSTEM_NAME = 'myFS';

    const int BLOCK_SIZE = 512;
    const long MAX_FILE_SIZE = (1 << 28) - 1; // do this to long for 64 * 32MB Dateien
    const int MAX_FILES = 64;

    const int ROOT_ADDRESS = 1;
    const int INODES_ADDRESS = 2;
    const int FIRST_FAT_ADDRESS = (MAX_FILES + INODES_ADDRESS);

    const int ADDRESS_COUNT_PER_FAT_BLOCK = (BLOCK_SIZE / 4);
    const long FAT_SIZE = ((MAX_FILE_SIZE * MAX_FILES) / BLOCK_SIZE) / ADDRESS_COUNT_PER_FAT_BLOCK;
    //const int FAT_SIZE = (MAX_FILE_SIZE / (BLOCK_SIZE * MAX_FILES)); 
    const int FIRST_DATA_ADDRESS = (FIRST_FAT_ADDRESS + FAT_SIZE);
}

#endif