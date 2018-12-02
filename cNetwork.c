#include "cNetwork.h"

//#define  MAXTRY  3

// client function to send package
int sendPackage(char *hostname, int port, Package *sent, Package *response) {

	int sd = UDP_Open(0);
    if(sd < -1)
    {
        perror("Error opening connection.\n");
        return -1;
    }

    struct sockaddr_in addr;
    int rc = UDP_FillSockAddr(&addr, hostname, port);
    if(rc < 0)
    {
        perror("Error looking up host.\n");
        return -1;
    }

    // set up variables for select() to check timeout
    fd_set fd;
    struct timeval t;
    struct sockaddr_in addr2;
    t.tv_sec=3;
    t.tv_usec=0;

    // iterate MAXTRY times to try sending the message
    //for(int i = 0; i < MAXTRY; i++) 
    while(1){
        FD_ZERO(&fd);       // Initializes the file descriptor set fdset to have zero bits for all file descriptors.
        FD_SET(sd, &fd);    // Sets the bit for the file descriptor fd in the file descriptor set fdset.
        
        // printf("inum: %d\n", sent->inum);
        // printf("ptype: %d\n", sent->ptype);
        // printf("name: %s\n", sent->name);

        rc = UDP_Write(sd, &addr, (char*)sent, sizeof(Package));
        if(select(sd+1, &fd, NULL, NULL, &t))
        {
            rc = UDP_Read(sd, &addr2, (char*)response, sizeof(Package));
            if(rc > 0)
            {
                // close the connection
                UDP_Close(sd);
                return 0;
            }
        }
    }

    return -1;
}
