#ifndef __DISKFS_H__
#define __DISKFS_H__
#include <stdbool.h>
#include "data_struct.h"


/*
DISKFS_Init() takes a file system image name and use it to initialize the file system. 
Return 0 if succeed, -1 if fail. 
*/
int DISKFS_Init(char* name);

/*
DISKFS_Lookup() takes the parent inode number (which should be the inode number of a directory) and looks up the entry name in it. 
The inode number of name is returned. Success: return inode number of name; failure: return -1. 
Failure modes: invalid pinum, name does not exist in pinum.
*/
int DISKFS_Lookup(int pinum, char *name);

/*
DISKFS_Stat() returns some information about the file specified by inum. 
Upon success, return 0, otherwise -1. The exact info returned is defined by DFS_Stat_t. 
Failure modes: inum does not exist.
*/
int DISKFS_Stat(int inum, DFS_Stat_t *stat);

/*
DISKFS_Write() writes a block of size 4096 bytes at the block offset specified by block . 
Returns 0 on success, -1 on failure. 
Failure modes: invalid inum, invalid block, not a regular file (you can't write to directories).
*/
int DISKFS_Write(int inum, void *buffer, int block);

/*
DISKFS_Read() reads a block specified by block into the buffer from file specified by inum. 
The routine should work for either a file or directory; directories should return data in the format specified by DFS_DirEnt_t. 
Success: 0, failure: -1. Failure modes: invalid inum, invalid block.
*/
int DISKFS_Read(int inum, void *buffer, int block);

/*
DISKFS_Creat() makes a file ( type == DFS_REGULAR_FILE) or directory ( type == DFS_DIRECTORY) 
in the parent directory specified by pinum of name name . 
Returns 0 on success, -1 on failure. 
Failure modes: pinum does not exist. If name already exists, return success.
*/
int DISKFS_Creat(int pinum, int type, char *name);

/*
DISKFS_Unlink() removes the file or directory name from the directory specified by pinum . 
0 on success, -1 on failure. 
Failure modes: pinum does not exist, pinum does not represent a directory, the to-be-unlinked directory is NOT empty. 
Note that the name not existing is NOT a failure by our definition.
*/
int DISKFS_Unlink(int pinum, char *name);

int DISKFS_Close(void);

#endif