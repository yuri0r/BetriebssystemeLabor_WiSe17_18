#ifndef inodeManager_h
#define inodeManager_h

#include "myfs.h"
#include "blockdevice.h"
#include "rootManager.h"
#include <stdlib.h>

struct InodeBlockStruct // Bytes: 256  + 3 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 32 + 32 = 344 @curvel this is outdated @yuri
{
    char fileName[256];                 // act of pure rebelion! (also 255 is just ugly) @yuri
    long fileSize;                      // size of file in bytes
    long usedBlocksCount;               // how many 512B Blocks
    unsigned int mode; // rwx
    long atime;                         // last access
    long mtime;                         // last modification
    long ctime;                         // last modification of status
    int firstFatEntry;                  // pointer to fat
    unsigned int userID;                // id Of user
    unsigned int groupID;               // id of group
    int index;
};

class InodeManager
{
  public:
    InodeManager();
    void createInode(BlockDevice *bd,
                     int inodeIndex,
                     char *fileName,
                     long fileSize,
                     long usedBlocksCount,
                     long atime,
                     long mtime,
                     long ctime,
                     int firstFatEntry,
                     unsigned int userID,
                     unsigned int groupID,
                     unsigned int mode);

    InodeBlockStruct* getInode(BlockDevice *bd,RootManager *rmgr, const char *fileName);
    char* getFileName(BlockDevice *bd, int index);
    InodeBlockStruct* getInodeByIndex(BlockDevice *bd, int index);
    void updateInode(BlockDevice *bd, InodeBlockStruct *inode);
    InodeBlockStruct* clearValidInode(BlockDevice *bd, RootManager *rmgr, const char *fileName);
};

#endif