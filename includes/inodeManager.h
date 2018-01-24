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
    unsigned long atime_nsec;
    long mtime;                         // last modification
    unsigned long mtime_nsec;
    long ctime;                         // last modification of status
    unsigned long ctime_nsec;
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
                     
    void createInode(BlockDevice *bd,
                     int inodeIndex,
                     char *fileName,
                     long fileSize,
                     long usedBlocksCount,
                     long atime,
                     unsigned long atime_nsec,
                     long mtime,
                     unsigned long mtime_nsec,
                     long ctime,
                     unsigned long ctime_nsec,
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