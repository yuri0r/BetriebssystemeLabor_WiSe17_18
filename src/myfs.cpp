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

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

#define LOGF(fmt, ...) \
do { fprintf(this->logFile, fmt "\n", __VA_ARGS__); } while (0)

#define LOG(text) \
do { fprintf(this->logFile, text "\n"); } while (0)

#define LOGM() \
do { fprintf(this->logFile, "%s:%d:%s()\n", __FILE__, \
__LINE__, __func__); } while (0)

InodeBlockStruct* getValidInode(const char *fileName) {
    fileName++;
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    for (int i = 0; i < MAX_FILES; i++) {
        if (rmgr->isValid(bd, i)) {
            inode = imgr->getInodeByIndex(bd, i); 
            if (strcmp(inode->fileName ,fileName) == 0){
                return inode;
            }
        }
    }

    free(inode);
    return NULL;
}

InodeBlockStruct* clearValidInode(const char *fileName) {
    fileName++;
    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);

    for (int i = 0; i < MAX_FILES; i++) {
        if (rmgr->isValid(bd, i)) {
            inode = imgr->getInodeByIndex(bd, i); 
            if (strcmp(inode->fileName ,fileName) == 0){
                rmgr->setInode(bd, i, false);
                return inode;
            }
        }
    }

    free(inode);
    return NULL;
}

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
    LOGF("\n# Try to get attributs from: %s", path);
    LOGM();

    if ( strcmp( path, "/" ) == 0 )
	{
        statbuf->st_uid = getuid();
        statbuf->st_gid = getgid();
         
        statbuf->st_mode = S_IFDIR | 0555;
		statbuf->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
    else
    {
        InodeBlockStruct* inode = getValidInode(path); 
        if (inode != NULL) {
            LOGF("# Get inode data from: %s", inode->fileName);
            statbuf->st_uid = inode->userID;
            statbuf->st_gid = inode->groupID;
            statbuf->st_size = inode->fileSize;
            statbuf->st_atime = inode->atime;
            statbuf->st_ctime = inode->ctime;
            statbuf->st_mtime = inode->mtime;
            statbuf->st_mode = inode->mode;
            LOGF("# FirstFatEntry: %i", inode->firstFatEntry);
            LOG("# Get atrributs went successfull");
        } else {
            LOG("# File does not exist!");
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
    inode = getValidInode(path);

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
        return ENOSPC;
    } else {
        LOG("# EEXIST (File with same name exists already)");
        free (inode);
        return EEXIST;
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
    inode = clearValidInode(path); 

    if (inode != NULL) {
        int currentFatAddress = inode->firstFatEntry;
        
        do {
            currentFatAddress = fmgr->readAndClearEntry(bd, currentFatAddress);
        } while (currentFatAddress != -1);
       
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
    LOGM();
    return 0;
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    //TODO
    LOGM();
    return 0;
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGF("\n# Trying to read %s, %u, %u", path, offset, size);
    LOGM();

    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = getValidInode(path); 

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
            LOGF("# Read textblock: %s", textBlock);
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
    LOGF("Buffer: %s", buf);
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

    InodeBlockStruct *inode = (InodeBlockStruct *)malloc(BLOCK_SIZE);
    inode = getValidInode(path); 

    if (inode == NULL) {
        LOG("# File not found");
        free(inode);
        return ENOENT;
    }

    int oldUsedBlockCount = inode->usedBlocksCount;

    LOGF("# Size to write = %u", size);
    int sizeCiel = 0;
    if ((size + offset) % BLOCK_SIZE == 0) {
        sizeCiel = size + offset;
    } else {
        sizeCiel = (((size + offset)/ BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }
    LOGF("# FileSize = %u", sizeCiel);

    char *textBlock = (char*)malloc(BLOCK_SIZE);

    int currentFatEntry = offset / BLOCK_SIZE; // Starts to write here
    LOGF("Start to write in block: %i", currentFatEntry);
    int blockCount = sizeCiel / BLOCK_SIZE; // How many blocks to write
    LOGF("Writes %i blocks", blockCount);

    int currentFatAddress = inode->firstFatEntry;
    int currentLastFatAddress = inode->firstFatEntry; // -1;

    LOGF("FirstFatEntry before expand= %i", inode->firstFatEntry);
    int usedBlockCountAfterWrite = currentFatEntry + blockCount;
    if (usedBlockCountAfterWrite > inode->usedBlocksCount) { // Expand FAT if needed
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
                if (inode->firstFatEntry == -1) {
                    inode->firstFatEntry = currentLastFatAddress;
                    LOG("Switched firstFatEntry");
                }
            }
        }
        inode->usedBlocksCount = usedBlockCountAfterWrite;
        LOGF("New usedBlockCount = %u", inode->usedBlocksCount);
        LOGF("FirstFatEntry after expand = %i", inode->firstFatEntry);
    }
    if (inode->fileSize < size + offset) {
            inode->fileSize = size + offset;
    }
    LOGF("New fileSize = %u", inode->fileSize);
    imgr->updateInode(bd, inode);

    currentFatAddress = inode->firstFatEntry;
    LOGF("currentFatEntry: %u", currentFatEntry);
    int currentBlockCount = 1;
    for (int i = 0; i < offset - BLOCK_SIZE; i = i + BLOCK_SIZE) {
        LOGF("read Fat: %u", i);
        currentFatAddress = fmgr->readFat(bd, currentFatAddress);
        currentBlockCount++;
    }
    
    int textBlockOffset = 0;
    bool firstBlockToWrite = true;
    int bufOffset = 0;
    int writeSize = 0;
    int restSize = size;
    LOGF("Buffer: %s", buf);
    for (int i = 0; i < blockCount; i++) {
        if (currentBlockCount <= inode->usedBlocksCount) {
            currentBlockCount++;
            textBlock = (char*)calloc(1, BLOCK_SIZE);
            LOGF("# Write address %i, Fat address %i", FIRST_DATA_ADDRESS + currentFatAddress, currentFatAddress);
            if (currentBlockCount <= oldUsedBlockCount + 1) {
                bd->read(FIRST_DATA_ADDRESS + currentFatAddress, textBlock);
                LOG("bd-read successfull");
            }
            LOGF("after bd-read \n i = %i", i);
            if (offset > 0) { // Dranhaengen
                if (firstBlockToWrite) {
                    textBlockOffset = offset % BLOCK_SIZE;
                    bufOffset = 0;
                } else {
                    textBlockOffset = 0;
                    bufOffset = bufOffset + writeSize;
                }
                if (restSize + textBlockOffset > BLOCK_SIZE) {
                    writeSize = BLOCK_SIZE - textBlockOffset;
                } else {
                    writeSize = restSize;
                }
                LOGF("CurrentBlockCount: %i", currentBlockCount);
                LOGF("textBlockOffset: %i, bufOffset: %i, writeSize: %i", textBlockOffset, bufOffset, writeSize);
                memcpy(textBlock + textBlockOffset, buf + bufOffset, writeSize);
                restSize = restSize - writeSize;
                firstBlockToWrite = false;
            } else { // Ueberschreiben
                LOG("CASE 3: Ueberschreiben");
                if (i == blockCount - 1) {
                    memcpy(textBlock, buf + (BLOCK_SIZE * i), size % BLOCK_SIZE);
                } else { // Ganzer Block
                    memcpy(textBlock, buf + (BLOCK_SIZE * i), BLOCK_SIZE);
                }
            }
           
            LOGF("after memcpy textBlock = %s", textBlock);
            bd->write(FIRST_DATA_ADDRESS + currentFatAddress, textBlock);
            LOG("after bd-write");
            // LOGF("## Read blockcount: %u", i);
            currentFatAddress = fmgr->readFat(bd, currentFatAddress);
            free(textBlock);
        }
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
    // TODO do we need it?
    return 0;
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
    // TODO do we need it?
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
        
    // you can get the containfer file name here:
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




