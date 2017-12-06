#include "superBlockManager.h"
#include "fsConfig.h"
using namespace fsConfig;

SuperBlockManager::SuperBlockManager() {}

void SuperBlockManager::init(BlockDevice* bd)
{
    SuperBlockStruct *sb = (SuperBlockStruct *)malloc(BLOCK_SIZE);

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

    if (sizeof(sb) > BLOCK_SIZE){
        printf("definition to large");
    } else {
        bd->write(0, (char *)sb);
    }
}