#ifndef _socket_h
#define _socket_h

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "init.h"

/**
 * Try to connect to the specified hostname and port.
 *
 * If successful, the socket file descriptor will be stored in the passed
 * peer_info struct.
 *
 * Returns 0 if successful or -1 if the connection fails.
 */
int make_socket_to_peer(struct peer_info *info);

#endif
