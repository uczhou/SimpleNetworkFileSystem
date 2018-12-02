#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mfs.h"
#include "cNetwork.h"
#include "data_struct.h"

char* 	server;
int 	serverPort;
int 	init = 0;

int MFS_Init(char *hostname, int port) {
	if(port < 0 || strlen(hostname) < 1)
		return -1;
	server = malloc(strlen(hostname) + 1);
	strcpy(server, hostname);
	serverPort = port;
	init = 1;

	return 0;
}

int MFS_Lookup(int pinum, char *name){
	if(init == 0)
		return -1;
	
	if(strlen(name) > 251)	// name exceeds the length of 251
		return -1;

	Package p, response;
	p.inum = pinum;
	p.ptype = LOOKUP;
	strcpy(p.name, name);

	if( sendPackage(server, serverPort, &p, &response) < 0)
		return -1;

	return response.inum;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
	if(init == 0)
		return -1;

	Package p;
	Package response;
	p.inum = inum;
	p.ptype = STAT;

	if(sendPackage(server, serverPort, &p, &response) < 0)
		return -1;

	m->type 		= response.stat.type;
	m->size 		= response.stat.size;
	m->blocks 	= response.stat.blocks;

	return response.inum;
}

int MFS_Write(int inum, char *buffer, int block){
	if(init == 0)
		return -1;
	
	Package p;
	Package response;
	p.inum = inum;
	memcpy(p.buffer, buffer, BUFFER_SIZE);
	p.block = block;
	p.ptype = WRITE;
	
	if(sendPackage(server, serverPort, &p, &response) < 0)
		return -1;
	
	return response.inum;
}

int MFS_Read(int inum, char *buffer, int block){
	if(init == 0)
		return -1;
	
	Package p;
	Package response;
	p.inum = inum;
	p.block = block;
	p.ptype = READ;

	if(sendPackage(server, serverPort, &p, &response) < 0)
		return -1;

	if(response.inum > -1)
		memcpy(buffer, response.buffer, BUFFER_SIZE);
	
	return response.inum;
}

int MFS_Creat(int pinum, int type, char *name){
	if(init == 0)
		return -1;
	
	if(strlen(name) > 251)
		return -1;

	Package p;
	Package response;
	p.inum = pinum;
	p.type = type;
	p.ptype = CREAT;
	strcpy(p.name, name);
	
	if(sendPackage(server, serverPort, &p, &response) < 0)
		return -1;

	return response.inum;
}

int MFS_Unlink(int pinum, char *name){
	if(init == 0)
		return -1;
	
	if(strlen(name) > 251)
		return -1;
	
	Package p;
	Package response;
	p.inum = pinum;
	p.ptype = UNLINK;
	strcpy(p.name, name);
	
	if(sendPackage(server, serverPort, &p, &response) < 0)
		return -1;

	return response.inum;
}
