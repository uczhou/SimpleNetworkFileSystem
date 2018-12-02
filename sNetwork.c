#include "sNetwork.h"
#include <stdio.h>

// server function to listen message
void serverListen(int port) {
	int sd = UDP_Open(port);
	if(sd < 0)
	{
		// printf("Error opening socket on port %d\n", port);
		exit(1);
	}

    // printf("Starting server...\n");
    while (1) {
		struct sockaddr_in addr;
		Package p;
		int rc = UDP_Read(sd, &addr, (char *)&p, sizeof(Package));
		if (rc > 0) {
		    Package response;
		    // printf("inum: %d\n", p.inum);
		    // printf("ptype: %d\n", p.ptype);
		    // printf("name: %s\n", p.name);
		    switch(p.ptype){

		    	case LOOKUP :
		    		response.inum = DISKFS_Lookup(p.inum, p.name);
		    		break;

		    	case STAT :
		    		response.inum = DISKFS_Stat(p.inum, &(response.stat));
		    		break;

		    	case WRITE :
		    		response.inum = DISKFS_Write(p.inum, p.buffer, p.block);
		    		break;

		    	case READ:
		    		response.inum = DISKFS_Read(p.inum, response.buffer, p.block);
		    		break;

		    	case CREAT:
		    		response.inum = DISKFS_Creat(p.inum, p.type, p.name);
		    		break;

		    	case UNLINK:
		    		response.inum = DISKFS_Unlink(p.inum, p.name);
		    		break;

		    	default: break;

		    }
		    rc = UDP_Write(sd, &addr, (char*)&response, sizeof(Package));

		}
	}
}


