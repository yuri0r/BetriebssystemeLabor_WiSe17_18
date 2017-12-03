//
//  mk.myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "mkfs_myfs.h"

BlockDevice *bd = new BlockDevice(BLOCK_SIZE);

void mkfs_myfs::initSuperBlock()
{
    SuperBlock *sb = (SuperBlock *)malloc(BLOCK_SIZE);

    sb->fileSystemName = FILE_SYSTEM_NAME;
    sb->blockSize = BLOCK_SIZE;
    sb->maxFileSize = MAX_FILE_SIZE;
    sb->maxFiles = MAX_FILES;
    sb->fatSize = FAT_SIZE;
    sb->adressCounterPerFatBlock = ADDRESS_COUNT_PER_FAT_BLOCK;
    sb->rootAdress = ROOT_ADDRESS;
    sb->inodesAdress = INODES_ADDRESS;
    sb->firsFatAdress = FIRST_FAT_ADDRESS;
    sb->firstDataAdress = FIRST_DATA_ADDRESS;

    if (sizeof(sb) > BLOCK_SIZE)
    {
        printf("definition to large");
    }
    else
    {
        bd->write(0, (char *)sb);
    }
}

void mkfs_myfs::setInodeInRoot(int inodeIndex, bool active)
{
    RootBlock *rb = (RootBlock *)malloc(BLOCK_SIZE);

    bd->read(ROOT_ADDRESS, (char *)rb);

    rb->inodesAddress[inodeIndex] = active;

    bd->write(ROOT_ADDRESS, (char *)rb);

    free(rb);
}

void mkfs_myfs::createInode(int inodeIndex,
                 char *fileName,
                 long fileSize,
                 long usedBlocksCount,
                 unsigned int mode,
                 long atime,
                 long mtime,
                 long ctime,
                 int firstFatEntry,
                 unsigned int userID,
                 unsigned int groupID)
{
    InodeBlock *inode = (InodeBlock *)malloc(BLOCK_SIZE);

    strcpy(inode->fileName, fileName);
    inode->fileSize = fileSize;
    inode->usedBlocksCount = usedBlocksCount;
    inode->mode = mode;
    inode->atime = atime;
    inode->mtime = mtime;
    inode->ctime = ctime;
    inode->firstFatEntry = firstFatEntry;
    inode->userID = userID;
    inode->groupID = groupID;

    if (inodeIndex >= 0 && inodeIndex < (FIRST_FAT_ADDRESS - INODES_ADDRESS))
    {
        bd->write(inodeIndex + INODES_ADDRESS, (char *)inode);
    }
    else
    {
        std::cout << "ERROR not in Inode Space: " << inodeIndex << std::endl;
    }
}

void mkfs_myfs::writeFat(int start, int destination)
{
    int fatBlockCount = (start - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = (start - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    int destinationIndex = (destination - FIRST_DATA_ADDRESS) / ADDRESS_COUNT_PER_FAT_BLOCK + (destination - FIRST_DATA_ADDRESS) % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    fb->destination[destinationCount] = destinationIndex;

    bd->write(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);
}

int mkfs_myfs::readFat(int position)
{
    int fatBlockCount = position / ADDRESS_COUNT_PER_FAT_BLOCK;
    int destinationCount = position % ADDRESS_COUNT_PER_FAT_BLOCK;

    FatBlock *fb = (FatBlock *)malloc(BLOCK_SIZE);

    bd->read(FIRST_FAT_ADDRESS + fatBlockCount, (char *)fb);

    return fb->destination[destinationCount];
}

void mkfs_myfs::dataCreation(int argc, char *argv[])
{
    int addressCounter = 0;
    int firstEntry;
    int blocksUsed;

    for (int i = 2; i < argc; i++)
    {
        std::streampos size;
        std::ifstream file(argv[i], std::ios::in | std::ios::binary | std::ios::ate); //openfile
        if (file.is_open())
        {
            size = file.tellg();
            char *filebuffer = (char *)malloc(size); //save file localy
            file.seekg(0, std::ios::beg);
            file.read(filebuffer, size);
            file.close();
            firstEntry = addressCounter;
            blocksUsed = 1;
            for (int i = 0; i < size; i += BLOCK_SIZE)
            {
                char *filewriter = filebuffer + i;
                bd->write(FIRST_DATA_ADDRESS + addressCounter, filewriter);
                int j = i + BLOCK_SIZE;
                addressCounter++;
                if (j < size)
                {
                    writeFat(FIRST_DATA_ADDRESS + addressCounter, FIRST_DATA_ADDRESS + addressCounter + 1);
                    blocksUsed++;
                }
            }
            //set inode and root entries
            struct stat fs;
            stat(argv[i], &fs);
            setInodeInRoot(i - 2, true);
            createInode(i - 2,
                        argv[i],
                        fs.st_size,
                        blocksUsed,
                        0444,
                        fs.st_atime,
                        fs.st_mtime,
                        fs.st_ctime,
                        firstEntry,
                        fs.st_uid,
                        fs.st_gid);

            std::cout << "File " << i - 1 << ": \"" << argv[i] << "\", Size: " << size << "Byte" << std::endl;
            free(filebuffer);
        }
    }
}

int mkfs_myfs::main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... file n>";
        return 1;
    }

    std::cout << "ContainerFile: " << argv[1] << std::endl;

    bd->create(argv[1]); // argv[1] = containerPath

    initSuperBlock();

    dataCreation(argc, argv);

    return 0;
}
