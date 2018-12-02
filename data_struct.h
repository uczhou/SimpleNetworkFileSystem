#ifndef __DATA_h__
#define __DATA_h__

// a copy of MFS_Stat_t but used in server side
typedef struct __DFS_Stat_t {
    int type;   
    int size;   
    int blocks; 
} DFS_Stat_t;

/* ------------------------------------- */

#define BUFFER_SIZE (4096)
#define MAX_NAME_SIZE (252)

typedef enum {
	INIT = 0,	
	LOOKUP,
	STAT,
	WRITE,
	READ,
	CREAT,
	UNLINK,
} Pack_type;

typedef struct package {
	Pack_type		ptype;
	int 			inum;
	int 			block;
	int 			type;
	DFS_Stat_t 		stat;

	char name[MAX_NAME_SIZE];
	char buffer[BUFFER_SIZE];
} Package; 

/* ------------------------------------- */

#define BLOCK_NUM 4096
#define INODE_NUM 4096
#define POINTER_NUM 10
#define ROOT_INODE 0
#define ENT_NUM_PER_BLOCK 16
#define INODE_PER_BLOCK 64
#define FILENAME_SIZE 252
#define DISK_BLOCK_SIZE 4096
#define DFS_DIRECTORY 0
#define DFS_REGULAR_FILE 1

typedef struct __Inode_t {
	int type;
	int size;
	int blocks;
	int direct[POINTER_NUM];
	int padding[3];
}Inode_t;

typedef struct __DirBlock_t {

	int inums[ENT_NUM_PER_BLOCK];
	char names[ENT_NUM_PER_BLOCK][FILENAME_SIZE];
	
}DirBlock_t;

typedef struct __DirEnt_t {
    int  inum;      // inode number of entry (-1 means entry not used)
    char name[252]; // up to 252 bytes of name in directory (including \0)
} DirEnt_t;

typedef struct __InodeBlock_t {
	Inode_t inodes[INODE_PER_BLOCK];
}InodeBlock_t;

typedef struct  __Superblock_t {
	int magic;
	int nblocks;
	int ninodes;
	int nmetablocks;
} Superblock_t;

#endif // __DATA_h__