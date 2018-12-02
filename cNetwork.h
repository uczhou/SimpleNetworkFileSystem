#ifndef __CNETWORK_H__
#define __CNETWORK_H__

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "udp.h"
#include "data_struct.h"

// client function to send package
int sendPackage(char *hostname, int port, Package *sent, Package *response);


#endif  // __CNETWORK_H__