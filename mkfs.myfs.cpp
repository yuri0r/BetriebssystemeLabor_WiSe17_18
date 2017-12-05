//
//  mk.myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define MAX_FILE_SIZE 2^31
#define MAX_FILES 64

#define SUPER_BLOCK_ADRESS 0
#define SUPER_BLOCK_SIZE 1
#define SUPER_BLOCK_NAME "myFS"

#define SUPER_BLOCK_NAME_POSITION 0
#define SUPER_BLOCK_BLOCK_SIZE_POSITION 32
#define SUPER_BLOCK_MAX_FILE_SIZE_POSITION 64
#define SUPER_BLOCK_MAX_FILES_POSITION 96
#define SUPER_BLOCK_ROOT_ADRESS_POSITION 128
#define SUPER_BLOCK_INODES_POSITION 160
#define SUPER_BLOCK_FAT_ADRESS_POSITION 192
#define SUPER_BLOCK_DATA_ADRESS_POSITION 224

#define ROOT_ADRESS 1
#define ROOT_SIZE 1

#define INODES_ADRESS 2
#define INODES_SIZE MAX_FILES

#define INODES_NAME_POSITION 0
#define INODES_SIZE_POSITION 255
#define INODES_OWNERID_POSITION 263
#define INODES_GROUPID_POSITION 267
#define INODES_PERMISSIONS_POSITION 271
#define INODES_ATIME_POSITION 274
#define INODES_MTIME_POSITION 299
#define INODES_CTIME_POSITION 324
#define INODES_ADDRESS_POSITION 349

#define FAT_ADRESS  65
#define FAT_SIZE MAX_FILE_SIZE / BLOCK_SIZE * MAX_FILES

#define DATA_ADRESS FAT_ADRESS + FAT_SIZE
#define DATA_SIZE //? in dont rly want to build a 128gb container file.... (@yuri)

int main(int argc, char *argv[]) {

    char* containerPath;
    struct stat metadata;
    
    // TODO: Implement file system generation & copying of files here
    /**
     * verifies, that number of arguments is legal
     * prints number of arguments
     * creates new file- container in path
     * gives back the address of each container- file
     *
     */
    if (argc < 2) {
        std::cout << "usuage ./mkfs.myfs <container file> <file1 file2 file3 ... file n>";
        return 1;
    }
    std::cout << "Argument count: " << argc << std::endl;
    
    containerPath = argv[1];
    BlockDevice *bd  = new BlockDevice(BLOCK_SIZE);
    bd->create(containerPath);

    for(int i = 0; i < argc; i++){
        std::cout << "Argument " << i << ": "  <<  argv[i] << std::endl;
        bd->write(DATA_ADRESS + i, argv[i]);         //argv[] can not exceed one block in size
    }

    // TODO create Superblock
    /**
     * allocates space for SuperBlock
     * fills SuperBlock with content
     */
    char* superBlockContent  = "our nice super file system of doom and citty cats <333";
    char* buffer = (char*)malloc(BLOCK_SIZE);

    for(int i = 0; i < strlen(superBlockContent); i++) {
            buffer[i] = superBlockContent[i];
    }

    bd->write(0,buffer);

    // End of create Superblock

    // TODO create FAT

    // End of create FAT

    // TODO create INODES
    /**
     * extracts filenames from arguments
     * verifies filename validity
     *
     * allocates space for inode content and fills it with the required data
     *
     */
    for (int i = 2; i < argc; i++){
        char* name = argv[i]; 

        if(stat(argv[i],&metadata)==-1){     //reading metadata documentation to stat():https://linux.die.net/man/2/stat
            std::cout<<name<<" is not a valid file name"<<std::endl;
            exit(EXIT_SUCCESS);
        }
        //all needed data, including conversion to char* if needed
        char const* size = (std::to_string(metadata.st_size)).c_str();      //Filesize in Bytes (long long, 8 bytes)
        char const* ownerId = (std::to_string(metadata.st_uid)).c_str();    //Id of the Owner    (long, 4 bytes)
        char const* groupId = (std::to_string(metadata.st_gid)).c_str();    //Group Id of the Owner (long)
        char* permissions = "r--";                                          //User Permissions: r read, w write, d delete (- right not granted)
        char const* accesstime = ctime(&metadata.st_atime);                //time of last file access
        char const* modificationtime = ctime(&metadata.st_mtime);           //time of last file modification
        char const* changetime = ctime(&metadata.st_ctime);                 //time of last status change

        char* inodeContent = (char*)malloc(BLOCK_SIZE);     //space to assemble content that is about to be writen
    //assemble Inodecontents;                                   
        for(int i = INODES_NAME_POSITION; i < strlen(name); i++) {
           inodeContent[i] = name[i];  
        }
        for(int i = INODES_SIZE_POSITION; i < (INODES_SIZE_POSITION + strlen(size)); i++) {
            inodeContent[i] = size[i - INODES_SIZE_POSITION];  
        }
        for(int i = INODES_OWNERID_POSITION; i < (INODES_OWNERID_POSITION + strlen(ownerId)); i++) {
            inodeContent[i] = ownerId[i - INODES_OWNERID_POSITION];  
        }
        for(int i = INODES_GROUPID_POSITION; i < (INODES_GROUPID_POSITION + strlen(groupId)); i++) {
            inodeContent[i] = groupId[i - INODES_GROUPID_POSITION];  
        }
        for(int i = INODES_PERMISSIONS_POSITION; i < (INODES_PERMISSIONS_POSITION + strlen(permissions)); i++) {
            inodeContent[i] = permissions[i - INODES_PERMISSIONS_POSITION];  
        }
        for(int i = INODES_ATIME_POSITION; i < (INODES_ATIME_POSITION + strlen(accesstime)); i++) {
            inodeContent[i] = accesstime[i - INODES_ATIME_POSITION];  
        }
        for(int i = INODES_MTIME_POSITION; i < (INODES_MTIME_POSITION + strlen(modificationtime)); i++) {
            inodeContent[i] = modificationtime[i - INODES_MTIME_POSITION];  
        }
        for(int i = INODES_CTIME_POSITION; i < (INODES_CTIME_POSITION + strlen(changetime)); i++) {
            inodeContent[i] = changetime[i - INODES_CTIME_POSITION];  
        }
    //write Inode
        bd->write(INODES_ADRESS + (i-2), inodeContent);
    }
    // End of create INODES

    // TODO create DATA

    // End of create DATA

    // TODO Calculate size of Binary file
        
    return 0;
}
