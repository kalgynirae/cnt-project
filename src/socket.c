#include "socket.h"

#define PORT_DIGITS 5

int make_socket_to_peer(struct peer_info *info)
{
    int s;  // for temporarily storing a function's return status

    /*
     * Set options for the upcoming call to getaddrinfo()
     */
    struct addrinfo hints;
    // First, make sure the struct is all zeros.
    memset(&hints, 0, sizeof(hints));
    // We want IPv4 addresses
    hints.ai_family = AF_INET;
    // This will be a TCP socket.
    hints.ai_socktype = SOCK_STREAM;
    // Dunno what this one means
    hints.ai_flags = AI_PASSIVE;

    // We need to pass the port as a string... go figure.
    char port[PORT_DIGITS + 1];
    snprintf(port, PORT_DIGITS + 1, "%d", info->port);

    /*
     * Call getaddrinfo() and check for success
     */
    struct addrinfo *result;
    s = getaddrinfo(info->hostname, port, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    /*
     * getaddrinfo() returns a linked list of address structures. Try each
     * address until we successfully connect().
     *
     * This code is based on the example in the getaddrinfo man page.
     */
    struct addrinfo *rp;
    int socket_fd;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        // Try to open the socket
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)
            continue;  // socket() failed, try next address

        // Try to connect() to the peer
        s = connect(socket_fd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0)
            break;  // success!
        else
            close(socket_fd);  // close the socket and try next address
    }

    if (rp == NULL)
    {
        fprintf(stderr, "Could not connect\n");
        return -1;
    }

    // Free the linked list from earlier
    freeaddrinfo(result);

    // Update the peer_info
    info->socket_fd = socket_fd;
    //info->state = ??;

    return 0;
}
