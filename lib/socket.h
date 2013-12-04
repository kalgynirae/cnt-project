#ifndef _socket_h
#define _socket_h

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "init.h"

#define OPEN_SOCKET_NEITHER 0
#define OPEN_SOCKET_LISTEN 1
#define OPEN_SOCKET_CONNECT 2

/*
 * Open a socket and bind or connect as requested.
 *
 * Returns a socket descriptor if successful or -1 on error.
 */
int open_socket(char *hostname, char *port, int connect_or_bind);

/*
 * Try to connect to the peer described by the given peer_info.
 *
 * If successful, the socket file descriptor will be stored in the passed
 * peer_info struct.
 *
 * Returns 0 if successful or -1 if the connection fails.
 */
int make_socket_to_peer(struct peer_info *info);

/*
 * Try to open a socket and listen for connections on the given port.
 *
 * If successful, the socket file descriptor will be stored in the passed
 * peer_info struct.
 *
 * Returns the socket descriptor or -1 on error.
 */
int open_socket_and_listen(char *port);

#endif
