#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "diskfs.h"
#include "utils.h"

#define DISKFS_MAGIC 123456
#define META_BLOCKS 65

static Superblock_t *superblock;
static Bitmap_t inode_bitmap;
static Bitmap_t block_bitmap;
static Inode_t inode_list[INODE_NUM];
static int disk_fd = -1;
static int debug = 0;

// update metadata information, if inum >=0, update with specific inode block, otherwise, update all inodes.
int update_metadata(int inum) {

	// seek the first block and write the superblock, bitmaps into disk
	lseek(disk_fd, 0, SEEK_SET);
	
	if (debug) {
		printf("Print bitmaps in update_metadata.\n");
	}

	write(disk_fd, superblock, sizeof(Superblock_t));

	write(disk_fd, inode_bitmap.bitmap, INODE_NUM / 8);

	write(disk_fd, block_bitmap.bitmap, BLOCK_NUM / 8);

	// seek the second block and write the inode list into disk
	lseek(disk_fd, DISK_BLOCK_SIZE, SEEK_SET);

	if (inum < 0) {
		
		for (int i = 0; i < 64; i++) {
			write(disk_fd, &inode_list[i * 64], DISK_BLOCK_SIZE);
		}

	}else {

		write(disk_fd, &inode_list[ (inum / 64) * 64], DISK_BLOCK_SIZE);
	}

	if (debug) {
		Superblock_t *tmp = malloc(sizeof(Superblock_t));
		Bitmap_t imap;
		Bitmap_t bmap;

		lseek(disk_fd, 0, SEEK_SET);
		read(disk_fd, tmp, sizeof(Superblock_t));
		read(disk_fd, imap.bitmap, 512);
		read(disk_fd, bmap.bitmap, 512);
	}

	return 0;
}

// Check if block is valid for read or write
int validate_metadata(int inum, int block) {

	if (inum < 0 || inum >= INODE_NUM || !get_bit_value(&inode_bitmap, inum)) {
		//printf("Error: Invalid inode number.\n");
		return -1;
	}
	
	if (block < 0 || block >= POINTER_NUM || block > inode_list[inum].blocks) {
		//printf("Error: Invalid block.\n");
		return -1;
	}

	return 0;
}

// Unset inode content
void inode_unset(int inum) {

	for(int i=0; i < inode_list[inum].blocks; i++){
		unset_bitmap(&block_bitmap, inode_list[inum].direct[i] - META_BLOCKS);
		inode_list[inum].direct[i] = 0;
	}
		
	inode_list[inum].type = 0;
	inode_list[inum].size = 0;
	inode_list[inum].blocks = 0;

	unset_bitmap(&inode_bitmap, inum);

}

// Read directory block
void dir_read(int inum, DirBlock_t *dirblock, int block) {

	lseek(disk_fd, inode_list[inum].direct[block] * DISK_BLOCK_SIZE, SEEK_SET);
	read(disk_fd, dirblock, DISK_BLOCK_SIZE);

}

// Write the first directory block
int write_dirblock(int inum) {
	DirBlock_t dirblock;

	for (int i = 0; i < ENT_NUM_PER_BLOCK; i++) {
		dirblock.inums[i] = -1;
		memset(dirblock.names[i], 0, 252);
	}

	dirblock.inums[0] = inum;
	dirblock.inums[1] = inum;
	strcpy(dirblock.names[0], ".\0");
	strcpy(dirblock.names[1], "..\0");

	lseek(disk_fd, inode_list[inum].direct[0] * DISK_BLOCK_SIZE, SEEK_SET);
	ssize_t ret = write(disk_fd, &dirblock, DISK_BLOCK_SIZE);

	if (ret == -1) return ret;
	else return 0;
}

// lookup the file/directory name in specified block
int fs_lookup(int pinum, char *name, int block, int size) {

	if (debug){
		printf("fs_lookup file: %s\n", name);
	}

	DirBlock_t dirblock;

	dir_read(pinum, &dirblock, block);

	int ent_num = size;

	if (size > ENT_NUM_PER_BLOCK) {
		ent_num = ENT_NUM_PER_BLOCK;
	}

	int i = 0;
	while(i < ent_num && strcmp(dirblock.names[i], name) != 0) {
		i++;
	}

	if (i < ent_num) {
		return dirblock.inums[i];
	}else {
		return -1;
	}
}

int DISKFS_Init(char* name) {
	// used for debug
	if (debug) {
		printf("Testing DISKFS_Init: filename = %s\n", name);
	}

	// used for debug
	int ret_val = 0;

	// Create superblock and initialize it
	superblock = malloc(sizeof(Superblock_t));

	memset(inode_bitmap.bitmap, 0, sizeof(char) * ((INODE_NUM + 7) / 8));
	memset(block_bitmap.bitmap, 0, sizeof(char) * ((BLOCK_NUM + 7) / 8));

	// used for debug
	if (debug) {
		printf("Print bitmaps in DISKFS_Init before initialization.\n");
	}
	
	// Create inode inode_list

	disk_fd = open(name, O_RDWR);

	// used for debug
	if (debug) {
		printf("disk_fd: %d\n", disk_fd);
	}
	
	// If file is already created.
	if (disk_fd >= 0) {
		//Initialize variables

		lseek(disk_fd, 0, SEEK_SET);
		read(disk_fd, superblock, sizeof(Superblock_t));
		read(disk_fd, inode_bitmap.bitmap, 512);
		read(disk_fd, block_bitmap.bitmap, 512);

		// Read inode list
		lseek(disk_fd, DISK_BLOCK_SIZE, SEEK_SET);

		for (int i = 0; i < 64; i++) {

			read(disk_fd, &inode_list[i * INODE_PER_BLOCK], DISK_BLOCK_SIZE);

		}

		// used for debug
		if (debug) {
			printf("Superblock meta: magic: %d, nblocks: %d, ninodes: %d, nmetablocks: %d\n", 
			superblock->magic, superblock->nblocks, superblock->ninodes, superblock->nmetablocks);
			printf("Print bitmaps in DISKFS_Init after initialization.\n");
		}
		// If file does not exist.
	}else {

		disk_fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
		
		if (disk_fd == -1) {
			// printf("Error: disk initialization failed.\n");
			return -1;
		}

		if (debug) {
			printf("Create file %s\n", name);
			ret_val = 1;
		}
		 

		ftruncate(disk_fd, (META_BLOCKS + BLOCK_NUM) * DISK_BLOCK_SIZE);

		// Initialize superblock
		superblock->magic = DISKFS_MAGIC;
		superblock->ninodes = INODE_NUM;
		superblock->nblocks = BLOCK_NUM;
		superblock->nmetablocks = META_BLOCKS;

		// Create root directory
		inode_list[ROOT_INODE].type = DFS_DIRECTORY;
		inode_list[ROOT_INODE].size = sizeof(DirEnt_t) * 2;
		inode_list[ROOT_INODE].blocks = 1;
		inode_list[ROOT_INODE].direct[0] = META_BLOCKS;

		set_bitmap(&inode_bitmap, 0);

		set_bitmap(&block_bitmap, 0);

		write_dirblock(0);

		update_metadata(-1);

		fsync(disk_fd);

	}

	return ret_val;
}


int DISKFS_Lookup(int pinum, char *name) {

	// check if pinum is valid
	if (pinum < 0 || pinum >= INODE_NUM || inode_list[pinum].type != DFS_DIRECTORY || inode_list[pinum].size == 0) {

		return -1;

	}

	int inum = -1;

	int ent_num = inode_list[pinum].size / sizeof(DirEnt_t);

	for (int i = 0; i < inode_list[pinum].blocks; i++) {
		
		inum = fs_lookup(pinum, name, i, ent_num);

		if (inum >= 0) return inum;
		
		ent_num -= ENT_NUM_PER_BLOCK;
	}

	return inum;

}



int DISKFS_Stat(int inum, DFS_Stat_t *stat) {

	if (inum < 0 || inum >= INODE_NUM || !get_bit_value(&inode_bitmap, inum)) {
		return -1;
	}

	stat->type = inode_list[inum].type;
	stat->size = inode_list[inum].size;
	stat->blocks = inode_list[inum].blocks;

	return 0;

}


int DISKFS_Write(int inum, void *buffer, int block) {

	// Check metadata
	if (validate_metadata(inum, block) < 0 || inode_list[inum].type != DFS_REGULAR_FILE) {
		//printf("Error: Invalid write.\n");
		return -1;
	}

	// If file size increase
	if ((block + 1) * DISK_BLOCK_SIZE > inode_list[inum].size) {

		// Allocate new block
		int n = get_empty_bit(&block_bitmap);
		set_bitmap(&block_bitmap, n);
		inode_list[inum].direct[block] = n + META_BLOCKS;

		inode_list[inum].blocks++;

		// Update the block size
		inode_list[inum].size = (block + 1) * DISK_BLOCK_SIZE;

		

	}

	lseek(disk_fd, inode_list[inum].direct[block] * DISK_BLOCK_SIZE, SEEK_SET);

	write(disk_fd, buffer, DISK_BLOCK_SIZE);
	 
	update_metadata(inum);

	fsync(disk_fd);

	return 0;

}

int DISKFS_Read(int inum, void *buffer, int block) {

	// check metadata
	if (validate_metadata(inum, block) < 0 || block == inode_list[inum].blocks) {
		// printf("Error: Invalid write.\n");
		return -1;
	}

	int n = inode_list[inum].direct[block];

	// read directly to buffer if it's a regular file
	if (inode_list[inum].type == DFS_REGULAR_FILE) {
		
		lseek(disk_fd, n * DISK_BLOCK_SIZE, SEEK_SET);
		read(disk_fd, buffer, DISK_BLOCK_SIZE);

	}else {
		
		DirBlock_t dirblock;

		lseek(disk_fd, n * DISK_BLOCK_SIZE, SEEK_SET);
		read(disk_fd, &dirblock, DISK_BLOCK_SIZE);

		for(int i = 0; i < ENT_NUM_PER_BLOCK; i++) {

			((DirEnt_t *) buffer)[i].inum = dirblock.inums[i];
			strcpy(((DirEnt_t *) buffer)[i].name, dirblock.names[i]);
		}
		
	}

	return 0;

}

int DISKFS_Creat(int pinum, int type, char *name) {

	if (pinum < 0 || pinum >= INODE_NUM || !get_bit_value(&inode_bitmap, pinum) || inode_list[pinum].type == DFS_REGULAR_FILE) {
		// printf("Error: Invalid inode number.\n");
		return -1;

	}

	if (DISKFS_Lookup(pinum, name) != -1) {
		return 0;
	}

	// Create file or directory

	// 1. Create inode and allocate data block
	int i_num = get_empty_bit(&inode_bitmap);
	set_bitmap(&inode_bitmap, i_num);

	

	inode_list[i_num].blocks = 0;
	//

	// Create regular file
	if (type == DFS_REGULAR_FILE) {
		inode_list[i_num].type = DFS_REGULAR_FILE;
		inode_list[i_num].size = 0;

	// Create directory
	}else {
		int b_num = get_empty_bit(&block_bitmap);
		set_bitmap(&block_bitmap, b_num);
		inode_list[i_num].blocks = 1;
		inode_list[i_num].direct[0] = b_num + META_BLOCKS;

		write_dirblock(i_num);

		inode_list[i_num].type = DFS_DIRECTORY;
		inode_list[i_num].size = sizeof(DirEnt_t) * 2;
		
	}

	// Update parent directory

	// If all the allocated blocks are used up
	if (inode_list[pinum].size == inode_list[pinum].blocks * DISK_BLOCK_SIZE) {

		// Allocate a new block
		int n = get_empty_bit(&block_bitmap);
		set_bitmap(&block_bitmap, n);

		inode_list[pinum].blocks++;
		inode_list[pinum].direct[inode_list[pinum].blocks - 1] = n + META_BLOCKS;
		inode_list[pinum].size += sizeof(DirEnt_t);

		DirBlock_t dirblock;

		dirblock.inums[0] = i_num;
		memset(dirblock.names[0], 0, 252);
		strcpy(dirblock.names[0], name);

		lseek(disk_fd, (n + META_BLOCKS) * DISK_BLOCK_SIZE, SEEK_SET);
		write(disk_fd, &dirblock, DISK_BLOCK_SIZE);

	}else {

		// Create a directory
		int offset = (inode_list[pinum].size / sizeof(DirEnt_t)) % ENT_NUM_PER_BLOCK;;
		//printf("offset: %d\n", offset);
		DirBlock_t dirblock;

		dir_read(pinum, &dirblock, inode_list[pinum].blocks - 1);

		dirblock.inums[offset] = i_num;
		memset(dirblock.names[offset], 0, 252);
		strcpy(dirblock.names[offset], name);

		lseek(disk_fd, inode_list[pinum].direct[inode_list[pinum].blocks - 1] * DISK_BLOCK_SIZE, SEEK_SET);
		write(disk_fd, &dirblock, DISK_BLOCK_SIZE);

		inode_list[pinum].size += sizeof(DirEnt_t);

	}
	
	update_metadata(-1);

	fsync(disk_fd);

	return 0;

}


int DISKFS_Unlink(int pinum, char *name) {
	/*
	MFS_Unlink() removes the file or directory name from the directory specified by pinum . 0 on success, -1 on failure. 
	Failure modes: pinum does not exist, pinum does not represent a directory, the to-be-unlinked directory is NOT empty. 
	Note that the name not existing is NOT a failure by our definition (think about why this might be).
	*/
	if (pinum < 0 || pinum >= INODE_NUM || !get_bit_value(&inode_bitmap, pinum) || inode_list[pinum].type == DFS_REGULAR_FILE) {
		// printf("Error: Invalid inode number.\n");
		return -1;
	}

	//int i_num = DISKFS_Lookup(pinum, name);

	int i_num = -1;

	int block = -1;

	int ent_num = inode_list[pinum].size / sizeof(DirEnt_t);

	for (int i = 0; i < inode_list[pinum].blocks; i++) {
		
		i_num = fs_lookup(pinum, name, i, ent_num);

		if (i_num >= 0) {
			block = i;
			break;
		}
		
		ent_num -= ENT_NUM_PER_BLOCK;
	}

	if (i_num == -1) return 0;

	if (inode_list[i_num].type == DFS_REGULAR_FILE || inode_list[i_num].size == sizeof(DirEnt_t) * 2) {
		// Remove the file
		inode_unset(i_num);

	}else {

		return -1;
				
	}

	int lastblock = (inode_list[pinum].size / sizeof(DirEnt_t)) % ENT_NUM_PER_BLOCK;

	if (lastblock == 0) {
		lastblock = ENT_NUM_PER_BLOCK;
	}
	
	if (debug) {
		printf("block is: %d\n", block);
		printf("last block is: %d\n", lastblock);
	}

	DirBlock_t dirblock;
	
	dir_read(pinum, &dirblock, block);

	// 1. Entry is in the last block
	if (block == inode_list[pinum].blocks - 1) {

		if (lastblock == 1) {
			unset_bitmap(&block_bitmap, inode_list[pinum].direct[block] - META_BLOCKS);
			inode_list[pinum].direct[block] = 0;
			inode_list[pinum].blocks --;
			inode_list[pinum].size -= sizeof(DirEnt_t);
		
		}else {

			int ent_pos = 0;

			while(ent_pos < lastblock && strcmp(dirblock.names[ent_pos], name) != 0) {
				ent_pos++;
			}
			
			if (ent_pos < lastblock - 1) {
				dirblock.inums[ent_pos] = dirblock.inums[lastblock - 1];		
				strcpy(dirblock.names[ent_pos], dirblock.names[lastblock - 1]);
			}

			dirblock.inums[lastblock - 1] = -1;
			memset(dirblock.names[lastblock - 1], 0 ,252);
			inode_list[pinum].size -= sizeof(DirEnt_t);



		}
		
		// 2. Entry is not in the last block
	}else {

		// Get the entry number
		int ent_pos = 0;

		while(ent_pos < ENT_NUM_PER_BLOCK && strcmp(dirblock.names[ent_pos], name) != 0) {
			ent_pos++;
		}
		
		// swap the last entry to the deleted position and delete the last entry.
		DirBlock_t tmp_dirblock;

		dir_read(pinum, &tmp_dirblock, inode_list[pinum].blocks - 1);

		if (lastblock == 1) {

			dirblock.inums[ent_pos] = tmp_dirblock.inums[0];
			strcpy(dirblock.names[ent_pos], tmp_dirblock.names[0]);

			unset_bitmap(&block_bitmap, inode_list[pinum].direct[inode_list[pinum].blocks - 1] - META_BLOCKS);
			inode_list[pinum].direct[inode_list[pinum].blocks - 1] = 0;
			inode_list[pinum].blocks--;
			inode_list[pinum].size -= sizeof(DirEnt_t);

		}else {

			dirblock.inums[ent_pos] = tmp_dirblock.inums[lastblock - 1];

			strcpy(dirblock.names[ent_pos], tmp_dirblock.names[lastblock - 1]);

			tmp_dirblock.inums[lastblock - 1] = 0;

			memset(tmp_dirblock.names[lastblock - 1], 0 ,252);

			inode_list[pinum].size -= sizeof(DirEnt_t);

			lseek(disk_fd, inode_list[pinum].direct[inode_list[pinum].blocks - 1], SEEK_SET);
			write(disk_fd, &tmp_dirblock, DISK_BLOCK_SIZE);
		}

	}

	lseek(disk_fd, inode_list[pinum].direct[block] * DISK_BLOCK_SIZE, SEEK_SET);
	write(disk_fd, &dirblock, DISK_BLOCK_SIZE);

	update_metadata(-1);

	fsync(disk_fd);

	return 0;

}

int DISKFS_Close(void) {
	if (disk_fd >= 0) {
		return close(disk_fd);
	}
	return 0;
}

