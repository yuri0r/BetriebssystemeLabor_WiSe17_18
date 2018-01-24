//
//  myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include <iostream>
#include <cstring>
#include <cmath>
#include <unistd.h>

#include "myfs.h"
#include "myfs-info.h"
#include "fsConfig.h"
#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"
#include "superBlockManager.h"
#include "inodeManager.h"
#include "fatManager.h"
#include "fsConfig.h"
#include "rootManager.h"
using namespace fsConfig;

MyFS* MyFS::_instance = NULL;
BlockDevice *bd = new BlockDevice(BLOCK_SIZE);
SuperBlockManager *sbmgr = new SuperBlockManager();
InodeManager *imgr = new InodeManager();
FatManager *fmgr = new FatManager();
RootManager *rmgr = new RootManager();
int openFiles = 0;

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

#define LOGF(fmt, ...) \
do { fprintf(this->logFile, fmt "\n", __VA_ARGS__); } while (0)

#define LOG(text) \
do { fprintf(this->logFile, text "\n"); } while (0)

#define LOGM() \
do { fprintf(this->logFile, "%s:%d:%s()\n", __FILE__, \
__LINE__, __func__); } while (0)

MyFS* MyFS::Instance() {
    if(_instance == NULL) {
        _instance = new MyFS();
    }
    return _instance;
}

MyFS::MyFS() {
    this->logFile= stderr;
}

MyFS::~MyFS() {
    
}

int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    //LOGF("\n# Try to get attributs from: %s", path);
    //LOGM();

    if ( strcmp( path, "/" ) == 0 )
	{
        statbuf->st_uid = getuid();
        statbuf->st_gid = getgid();
         
        statbuf->st_mode = S_IFDIR | 0555;
		statbuf->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
    else
    {
        InodeBlockStruct* inode = imgr->getInode(bd,rmgr,path); 
        if (inode != NULL) {
            LOGF("# Get inode data from: %s", inode->fileName);
            statbuf->st_uid = inode->userID;
            statbuf->st_gid = inode->groupID;
            statbuf->st_size = inode->fileSize;
            statbuf->st_atime = inode->atime;
            statbuf->st_atim.tv_nsec = inode->atime_nsec;
            statbuf->st_ctime = inode->ctime;
            statbuf->st_ctim.tv_nsec = inode->ctime_nsec;
            statbuf->st_mtime = inode->mtime;
            statbuf->st_mtim.tv_nsec = inode->mtime_nsec;
            statbuf->st_mode = inode->mode;
            //LOGF("# FirstFatEntry: %i", inode->firstFatEntry);
            //LOG("# Get atrributs went successfull");
        } else {
            //LOG("# File does not exist!");
            return -ENOENT;
        }
        statbuf->st_nlink = 1;
    }
    return 0;
}

int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGF("# Create File %s", path);
    LOGM();

    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = imgr->getInode(bd, rmgr, path);

    if (inode == NULL) {
        for (int index = 0; index < MAX_FILES; index++) {
            if (!rmgr->isValid(bd, index)) {
                LOG("Create inode");
                path++;
                imgr->createInode(bd, index, (char*)path, 0, 0,  time(0), time(0),
                            time(0), -1, getuid(), getgid(), mode);

                LOG("Set inode active in root block");
                rmgr->setInode(bd, index, true);

                LOG("Node created");
                free (inode);
                return 0;
            }
        }
        LOG("# ENOSPC (No free inode)");
        free (inode);
        return -ENOSPC;
    } else {
        LOG("# EEXIST (File with same name exists already)");
        free (inode);
        return -EEXIST;
    }
}

int MyFS::fuseMkdir(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseUnlink(const char *path) {
    LOGF("\n# Try to delete file: %s", path);
    LOGM();
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode =  imgr->clearValidInode(bd, rmgr, path); 

    if (inode != NULL) {
        int currentFatAddress = inode->firstFatEntry;
        
        while (currentFatAddress != -1) {
            currentFatAddress = fmgr->clearIndex(bd, currentFatAddress);
        }
       
        for (int i = 0; i < MAX_FILES; i++) {
            if (rmgr->isValid(bd, i)) {
                LOGF("# INODE %u is aktiv", i);
            }
        }

        LOGF("# Delete file: %s, successfully", path);
        return 0;
    }

    LOGF("# Couldnt delete file: %s", path);
    return ENOENT;
}

int MyFS::fuseRmdir(const char *path) {
    LOGM();
    return 0;
}

int MyFS::fuseSymlink(const char *path, const char *link) {
    LOGM();
    return 0;
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseLink(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGF("\n# Trying to truncate %s, to Size: %u", path, newSize);
    LOGM();

    // Get inode of file to truncate
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = imgr->getInode(bd, rmgr, path); 

    // If inode doesnt exist = File does not exist
    if (inode == NULL) {
        LOG("# File not found");
        free(inode);
        return ENOENT;
    }

    // Get FileSize rounded in 512 steps
    int roundedSize = 0;
    if (newSize % BLOCK_SIZE == 0) {
        roundedSize = newSize;
    } else {
        roundedSize = ((newSize/ BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }
    LOGF("# FileSize = %u", roundedSize);

    // Init more Paramas
    int blockCountAfterTruncate = roundedSize / BLOCK_SIZE; // How many blocks are needed
    LOGF("Needs %i blocks", blockCountAfterTruncate);

    int currentFatAddress = inode->firstFatEntry;
    int currentLastFatAddress = currentFatAddress;
    int fatPointer = currentFatAddress;
    int oldUsedBlockCount = inode->usedBlocksCount; // File Blockcount before write

    // Expand Fat if needed
    
    if (blockCountAfterTruncate > oldUsedBlockCount) {
        LOGF("FirstFatEntry before expand= %i", currentFatAddress);
        for (int i = 0; i < blockCountAfterTruncate; i++) {
            if (currentFatAddress != -1) {
                LOGF("CurrentFatAddress: %i", currentFatAddress);
                currentFatAddress = fmgr->readFat(bd, currentFatAddress);
                LOGF("CurrentFatAddress after read: %i", currentFatAddress);
                if (currentFatAddress != -1) {
                    LOG("changed currentLastFatAddress");
                    currentLastFatAddress = currentFatAddress;
                }
            } else {
                LOG("Expand FAT");
                LOGF("Try to expand, currentLastFatAddress: %i", currentLastFatAddress);
                currentLastFatAddress = fmgr->expand(bd, currentLastFatAddress);
                LOGF("CurrentLastAddress after expand: %i", currentLastFatAddress);
                if (currentLastFatAddress == -1) {
                    LOG("!!! NO FREE FAT ENTRY");
                    return -ENOSPC;
                }
                if (inode->firstFatEntry == -1) {
                    inode->firstFatEntry = currentLastFatAddress;
                    LOG("Switched firstFatEntry");
                }
            }
        }
        LOGF("New usedBlockCount = %u", inode->usedBlocksCount);
        LOGF("FirstFatEntry after expand = %i", inode->firstFatEntry);
        // End of: Expand Fat if needed
    } else if (blockCountAfterTruncate == oldUsedBlockCount){
        LOG("same amount of fatblocks needed");
    } else {
        LOG("fat has to be shortend");
        for (int blocks = 0; blocks < blockCountAfterTruncate - 1; blocks ++) {
            LOGF("jumped over fat entry %i",fatPointer);
            fatPointer = fmgr->readFat(bd,fatPointer);
        }
        if (blockCountAfterTruncate != 0) {
            fatPointer = fmgr->markEoF(bd,fatPointer);
        }
        LOGF("FatPointer: %i", fatPointer);
        LOGF("Blocks: %i, oldUsedBlockCount: %i", blockCountAfterTruncate, oldUsedBlockCount);
        while (fatPointer != -1) {
            LOGF("cleared fat entry %i",fatPointer);
            fatPointer = fmgr->clearIndex(bd, fatPointer);
        }
        LOG("End of shorten");
    } 
    
    // Update file Size
    inode->fileSize = newSize;
    LOGF("New fileSize = %u", inode->fileSize);

    inode->usedBlocksCount = blockCountAfterTruncate;
    LOGF("New usedBlockCount = %u", inode->usedBlocksCount);

    if (blockCountAfterTruncate == 0) {
        inode->firstFatEntry = -1;
    }

    LOGF("First Fat Entry: %i", inode->firstFatEntry);

    // Update write time 
    inode->ctime = time(0);
    inode->mtime = time(0);

    // Safe Inode changes 
    imgr->updateInode(bd, inode);
    return 0;
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    if (openFiles < MAX_FILES) {
        openFiles++;
        LOGF("Open files: %i", openFiles);
        return 0;
    } else {
        LOG("Cant open this file! %i files already open!");
        return -ENFILE;
    }
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGF("\n# Trying to read %s, %u, %u", path, offset, size);
    LOGM();

    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = imgr->getInode(bd, rmgr ,path); 

    if (inode == NULL) {
        LOG("# File not found");
        free(inode);
        return ENOENT;
    }

    int currentFatAddress = inode->firstFatEntry;
    if (currentFatAddress == -1) {
        LOG("# File is empty");
        free(inode);
        return 0;
    }

    LOGF("# Size to read = %u", size);
    int sizeCiel = 0;
    if (size % BLOCK_SIZE == 0) {
        sizeCiel = size;
    } else {
        sizeCiel = ((size / BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }
    LOGF("# SizeCiel to read = %u", sizeCiel);
    char *finalText = (char*)malloc(sizeCiel);
    char *textBlock = (char*)malloc(BLOCK_SIZE);

    int currentFatEntry = offset / BLOCK_SIZE; // Starts to read here
    int blockCount = sizeCiel / BLOCK_SIZE; // How many blocks to read
    int cantReadBlockCount = 0;

    LOG("# inode != null");
    int currentBlockCount = 1;
    if (currentFatEntry != 1) {
        for (int i = 0; i < currentFatEntry; i++) {
            //LOGF("read Fat: %u", i);
            currentFatAddress = fmgr->readFat(bd, currentFatAddress);
            currentBlockCount++;
        }
    }

    LOGF("# UsedBlockCount = %u", inode->usedBlocksCount);
    LOGF("# currentFat Address = %u", currentFatAddress);
    LOGF("# Blocks to read = %u", blockCount);
    // "currentFatAddress" is now the first Block to read real Data

    bool enxio = false;

    for (int i = 0; i < blockCount; i++) {
        if (currentBlockCount <= inode->usedBlocksCount) {
            currentBlockCount++;
            textBlock = (char*)calloc(1, BLOCK_SIZE);
            LOGF("# Read address %u, Fat address %u", FIRST_DATA_ADDRESS + currentFatAddress, currentFatAddress);
            bd->read(FIRST_DATA_ADDRESS + currentFatAddress, textBlock);
           // LOGF("# Read textblock: %s", textBlock);
            memcpy(finalText + (BLOCK_SIZE * i), textBlock, BLOCK_SIZE);
            currentFatAddress = fmgr->readFat(bd, currentFatAddress);
            free(textBlock);
        } else {
            cantReadBlockCount++;
            LOG("# Try to read a block out of range of usedBlocksCount");
            // enxio = true;
        }
    }
    LOG("# Finished read process");

    memcpy( buf, finalText + (offset % BLOCK_SIZE), size - (cantReadBlockCount * BLOCK_SIZE));
   // LOGF("Buffer: %s", buf);

    // Update write time 
    inode->atime = time(0);

    // Safe Inode changes 
    imgr->updateInode(bd, inode);

    free(inode);
    free(finalText);
    if (enxio) {
        return ENXIO;
    } else {
        return size - (cantReadBlockCount * BLOCK_SIZE);
    }
}

int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGF("\n# Trying to write %s, Offset: %u, Size: %u", path, offset, size);
    LOGM();

    // Get inode of file to write
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = imgr->getInode(bd, rmgr, path); 

    // If inode doesnt exist = File does not exist
    if (inode == NULL) {
        LOG("# File not found");
        free(inode);
        return ENOENT;
    }

    // Init Params
    int oldUsedBlockCount = inode->usedBlocksCount; // File Blockcount before write
    char *textBlock = (char*)malloc(BLOCK_SIZE); // Buffer for one dataBlock

    // Get FileSize rounded in 512 steps
    LOGF("# Size to write = %u", size);
    int roundedSize = 0;
    if ((size + offset) % BLOCK_SIZE == 0) {
        roundedSize = size + offset;
    } else {
        roundedSize = (((size + offset)/ BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }
    LOGF("# FileSize = %u", roundedSize);

    // Init more Paramas
    int currentFatEntry = offset / BLOCK_SIZE; // Start to write at this Fat entry
    LOGF("Start to write in block: %i", currentFatEntry);
    int blockCount = roundedSize / BLOCK_SIZE; // How many blocks are needed
    LOGF("Needs %i blocks", blockCount);

    int currentFatAddress = inode->firstFatEntry;
    int currentLastFatAddress = currentFatAddress;
    int fatPointer = currentFatAddress;

    // Expand Fat if needed
    
    int usedBlockCountAfterWrite = blockCount;
    if (usedBlockCountAfterWrite > oldUsedBlockCount) {
        LOGF("FirstFatEntry before expand= %i", currentFatAddress);
        for (int i = 0; i < usedBlockCountAfterWrite; i++) {
            if (currentFatAddress != -1) {
                LOGF("CurrentFatAddress: %i", currentFatAddress);
                currentFatAddress = fmgr->readFat(bd, currentFatAddress);
                LOGF("CurrentFatAddress after read: %i", currentFatAddress);
                if (currentFatAddress != -1) {
                    LOG("changed currentLastFatAddress");
                    currentLastFatAddress = currentFatAddress;
                }
            } else {
                LOG("Expand FAT");
                LOGF("Try to expand, currentLastFatAddress: %i", currentLastFatAddress);
                currentLastFatAddress = fmgr->expand(bd, currentLastFatAddress);
                LOGF("CurrentLastAddress after expand: %i", currentLastFatAddress);
                if (currentLastFatAddress == -1) {
                    LOG("!!! NO FREE FAT ENTRY");
                    return -ENOSPC;
                }
                if (inode->firstFatEntry == -1) {
                    inode->firstFatEntry = currentLastFatAddress;
                    LOG("Switched firstFatEntry");
                }
            }
        }
        LOGF("FirstFatEntry after expand = %i", inode->firstFatEntry);
        // End of: Expand Fat if needed
    } else if (usedBlockCountAfterWrite == oldUsedBlockCount){
        LOG("same amount of fatblocks needed");
    } else {
        LOG("fat has to be shortend");
        for (int blocks = 0; blocks < usedBlockCountAfterWrite - 1; blocks ++) {
            LOGF("jumped over fat entry %i",fatPointer);
            fatPointer = fmgr->readFat(bd,fatPointer);
        }
        if (usedBlockCountAfterWrite != 0) {
            fatPointer = fmgr->markEoF(bd,fatPointer);
        }
        LOGF("FatPointer: %i", fatPointer);
        LOGF("Blocks: %i, oldUsedBlockCount: %i", usedBlockCountAfterWrite, oldUsedBlockCount);
        while (fatPointer != -1) {
            LOGF("cleared fat entry %i",fatPointer);
            fatPointer = fmgr->clearIndex(bd, fatPointer);
        }
        LOG("End of shorten");
    } 
    
    // Update file Size
    inode->fileSize = size + offset;
    LOGF("New fileSize = %u", inode->fileSize);

    inode->usedBlocksCount = usedBlockCountAfterWrite;
    LOGF("New usedBlockCount = %u", inode->usedBlocksCount);

    if (usedBlockCountAfterWrite == 0) {
        inode->firstFatEntry = -1;
    }

    LOGF("First Fat Entry: %i", inode->firstFatEntry);

    // Update write time 
    inode->ctime = time(0);
    inode->mtime = time(0);

    // Safe Inode changes 
    imgr->updateInode(bd, inode);

    // Get first Address to write
    currentFatAddress = inode->firstFatEntry;
    LOGF("currentFatEntry: %u", currentFatEntry);
    int currentBlockCount = 1;
    for (int i = 0; i <= offset - BLOCK_SIZE; i = i + BLOCK_SIZE) {
        LOGF("read Fat: %u", i);
        currentFatAddress = fmgr->readFat(bd, currentFatAddress);
        currentBlockCount++;
    }
    
    // Init write params
    int textBlockOffset = 0;
    bool firstBlockToWrite = true;
    int bufOffset = 0;
    int writeSize = 0;
    int restSize = size;
    int i = 0;

    // Print complete data which to write
    //LOGF("Buffer: %s", buf);

    // Start to write
    while (restSize > 0) { // Still data to write 
        LOGF("# Write address %i, Fat address %i", FIRST_DATA_ADDRESS + currentFatAddress, currentFatAddress);

        // Set params for this loop
        textBlock = (char*)calloc(1, BLOCK_SIZE);

        if (firstBlockToWrite) { // First time in this loop
            textBlockOffset = offset % BLOCK_SIZE;
            bufOffset = 0;
        } else { // Not first time in this loop
            textBlockOffset = 0;
            bufOffset = bufOffset + writeSize;
        }

        // How much data to write in this loop
        if (restSize + textBlockOffset > BLOCK_SIZE) {
            writeSize = BLOCK_SIZE - textBlockOffset;
        } else {
            writeSize = restSize;
        }

        // If needed copy data from BlockDevice to textBlock
        if (firstBlockToWrite && currentBlockCount <= oldUsedBlockCount) {
            bd->read(FIRST_DATA_ADDRESS + currentFatAddress, textBlock);
            LOG("bd-read successfull");
        }

        // Copy right part of Buffer to TextBlock
        LOGF("CurrentBlockCount: %i", currentBlockCount);
        LOGF("textBlockOffset: %i, bufOffset: %i, writeSize: %i", textBlockOffset, bufOffset, writeSize);
        memcpy(textBlock + textBlockOffset, buf + bufOffset, writeSize);
        
        // Write textBlock to BlockDevice
       // LOGF("after memcpy textBlock = %s", textBlock);
        bd->write(FIRST_DATA_ADDRESS + currentFatAddress, textBlock);
        LOG("after bd-write");

        // Get next Fat Address to write
        currentFatAddress = fmgr->readFat(bd, currentFatAddress);

        // Set params for next loop
        free(textBlock);
        restSize = restSize - writeSize;
        firstBlockToWrite = false;
        currentBlockCount++;
    }

    LOGF("# Write to %s successfull", path);
    free(inode);
    return size;
}

int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    if (openFiles > 0) {
        openFiles--;
        LOGF("Open files: %i", openFiles);
        return 0;
    } else {
        LOG("Cant close more files! All files already closed!");
        return 0;
    }
}

int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    LOGM();
    return 0;
}

int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
    LOGM();
    return 0;
}

int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseRemovexattr(const char *path, const char *name) {
    LOGM();
    return 0;
}

int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();

	filler( buf, ".", NULL, 0 ); // Current Directory
	filler( buf, "..", NULL, 0 ); // Parent Directory

    LOG("\nShow files:");

    for (int i = 0; i < MAX_FILES; i++){
        if (rmgr->isValid(bd, i)) {
            char* fileName = imgr->getFileName(bd, i); 
            LOGF("# File%d: %s", i, fileName);
            filler(buf, fileName, NULL, 0);
        }
    }
	
	return 0;
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    // TODO
    return 0;
}

int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseInit(struct fuse_conn_info *conn) {

    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w");
    
    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
        return -1;
    }
    
    // turn of logfile buffering
    setvbuf(this->logFile, NULL, _IOLBF, 0);

    LOG("Starting logging...\n");
    LOGM();
        
    // you can get the container file name here:
    LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);
    
    // TODO : Enter your code here!
    bd->open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);
    sbmgr->load(bd);

    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

void MyFS::fuseDestroy() {
    LOGM();
}