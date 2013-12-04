#include "socket.h"

fd_set read_fds;
int max_fd;

int open_socket(char *hostname, char *port, int mode)
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

    /*
     * Call getaddrinfo() and check for success
     */
    struct addrinfo *result;
    s = getaddrinfo(hostname, port, &hints, &result);
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
        {
            // socket() failed, try next address
            continue;
        }

        if (mode == OPEN_SOCKET_CONNECT)
        {
            // Try to connect() to the peer
            s = connect(socket_fd, rp->ai_addr, rp->ai_addrlen);
            if (s == 0)
            {
                // Success!
                break;
            }
        }
        else if (mode == OPEN_SOCKET_LISTEN)
        {
            // Try to bind() to the socket
            int yes = 1;
            setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
            s = bind(socket_fd, rp->ai_addr, rp->ai_addrlen);
            if (s == 0)
            {
                s = listen(socket_fd, 0);
                if (s == 0)
                {
                    // Success!
                    break;
                }
            }
        }
        else
        {
            // Don't connect() or bind(). Guaranteed success at this point!
            break;
        }

        // Close the socket and try the next address
        close(socket_fd);
        continue;
    }

    if (rp == NULL)
    {
        fprintf(stderr, "open_socket(): Could not connect\n");
        return -1;
    }

    // Free the linked list from earlier
    freeaddrinfo(result);

    // Add the socket to the global set and increase max_fd
    FD_SET(s, &read_fds);
    if (max_fd < s)
    {
        max_fd = s;
    }

    return socket_fd;
}

int make_socket_to_peer(struct peer_info *info)
{
    // Open the socket; store it in the peer_info
    int s = open_socket(info->hostname, info->port, OPEN_SOCKET_CONNECT);
    if (s != -1)
    {
        fprintf(stderr, "assign socket %d to peer %d\n", s, info->peer_id);
        info->socket_fd = s;
        return 0;
    }
    return -1;
}

int open_socket_and_listen(char *port)
{
    return open_socket(NULL, port, OPEN_SOCKET_LISTEN);
}
