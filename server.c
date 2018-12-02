#include <stdio.h>
#include "sNetwork.h"
#include "diskfs.h"

// Usage: ./server [portnum] [file-system-image]

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        // printf("Usage: server server-port-number file-image\n");
        exit(1);
    }

    DISKFS_Init(argv[2]);
    serverListen(atoi(argv[1]));

    return 0;
}


