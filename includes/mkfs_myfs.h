// Incudes
#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// Defines
#define FILE_SYSTEM_NAME 'myFS'

#define BLOCK_SIZE 512
#define MAX_FILE_SIZE (2 ^ 32) - 1
#define MAX_FILES 64

#define ROOT_ADDRESS 1
#define INODES_ADDRESS 2
#define FIRST_FAT_ADDRESS MAX_FILES + INODES_ADDRESS

#define FAT_SIZE (MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES)
#define FIRST_DATA_ADDRESS (FIRST_FAT_ADDRESS + FAT_SIZE)
#define ADDRESS_COUNT_PER_FAT_BLOCK BLOCK_SIZE / 4

// Structs
struct SuperBlock
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

struct RootBlock
{
    // false = for inactive node
    // true  = active node
    bool inodesAddress[MAX_FILES] = {0};
};

struct InodeBlock // Bytes: 256  + 3 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 32 + 32 = 344 @curvel this is outdated @yuri
{
    char fileName[256];   // act of pure rebelion! (also 255 is just ugly) @yuri
    long fileSize;        // size of file in bytes
    long usedBlocksCount; // how many 512B Blocks
    unsigned int mode;    // rwx
    long atime;           // last access
    long mtime;           // last modification
    long ctime;           // last modification of status
    int firstFatEntry;    // pointer to fat
    unsigned int userID;  // id Of user
    unsigned int groupID; // id of group
};

struct FatBlock
{
    int destination[ADDRESS_COUNT_PER_FAT_BLOCK] = {};
};

class mkfs_myfs {
    private:
        int main(int argc, char *argv[]);
        void initSuperBlock();
        void dataCreation(int argc, char *argv[]);
        void createInode(int inodeIndex,
            char *fileName,
                long fileSize,
                long usedBlocksCount,
                unsigned int mode,
                long atime,
                long mtime,
                long ctime,
                int firstFatEntry,
                unsigned int userID,
                unsigned int groupID);
        void writeFat(int start, int destination);
        void setInodeInRoot(int inodeIndex, bool active);

    public:
        int readFat(int position);
};