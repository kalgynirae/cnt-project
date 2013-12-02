#include "peer_log.h"

/**
 * Functions used for logging events in the process
 */

//a connection was established between 2 peers, initiated by peer #from_peer
void log_connect(int from_peer, int to_peer)
{
    char log[80];

    snprintf(log, 80, 
            "Peer %d makes a connection to peer %d\n", from_peer, to_peer);
    write_log(from_peer, log);

    snprintf(log, 80, 
            "Peer %d is connected from Peer %d\n", to_peer, from_peer);
    write_log(to_peer, log);
}

//a peer has changed its preferred neighbors
void log_change_preferred(int peer, int num_preferred, int new_preferred[])
{
    char log[80];
    char neighbors[80] = "";
    char id[5];
    int i;

    for (i = 0 ; i < num_preferred - 1; i++)
    {
        sprintf(id, "%d", new_preferred[i]);
        strcat(neighbors, id);
        strcat(neighbors, ", ");
    }
    sprintf(id, "%d", new_preferred[i]);
    strcat(neighbors, id);          //last without trailing ','

    snprintf(log, 80, "Peer %d has the preferred neighbors %s\n", peer, neighbors);
            
    write_log(peer, log);
}

//a peer has optimistically unchoked unchoked_peer
void log_optimistic_unchoke(int peer, int unchoked_peer)
{
    char log[80];

    snprintf(log, 80, "Peer %d has the optimistically-unchoked neighbor %d\n",
            peer, unchoked_peer);

    write_log(peer, log);
}

//reciever got UNCHOKE message from sender
void log_unchoked_by(int reciever, int sender)
{
    char log[80];

    snprintf(log, 80, "Peer %d is unchoked by %d\n", reciever, sender);

    write_log(reciever, log);
}

//reciever got CHOKE message from sender
void log_recieve_choke(int reciever, int sender)
{
    char log[80];

    snprintf(log, 80, "Peer %d is choked by %d\n", reciever, sender);

    write_log(reciever, log);
}

//reciever got HAVE message from sender
void log_recieved_have(int reciever, int sender)
{
    char log[80];

    snprintf(log, 80, "Peer %d recieved 'have' message from %d\n",
            reciever, sender);

    write_log(reciever, log);
}

//reciever got INTERESTED message from sender
void log_recieved_interested(int reciever, int sender)
{
    char log[80];

    snprintf(log, 80, "Peer %d recieved 'interested' message from %d\n",
            reciever, sender);

    write_log(reciever, log);
}

//reciever got NOT_INTERESTED message from sender
void log_recieved_not_interested(int reciever, int sender)
{
    char log[80];

    snprintf(log, 80, "Peer %d recieved 'not interested' message from %d\n",
            reciever, sender);

    write_log(reciever, log);
}

//peer has finished downloading piece
void log_downloaded_piece(int peer, int piece)
{
    char log[80];

    snprintf(log, 80, "Peer %d has downloaded the piece %d\n", peer, piece);

    write_log(peer, log);
}

//peer has finished all pieces of file
void log_downloaded_file(int peer)
{
    char log[80];

    snprintf(log, 80, "Peer %d has downloaded the complete file\n", peer);

    write_log(peer, log);
}

//write a log to the corresponding peer log file
void write_log(int peer, char* log)
{
    FILE* fp;
    char filename[LOG_FILENAME_LEN];
    snprintf(filename, LOG_FILENAME_LEN + 1, LOG_FILENAME, peer);

    //open in append mode
    if ((fp = fopen(filename, "a")) == NULL) {
        fprintf(stderr, "write_log(): Error opening log file for peer %d\n",
                peer);
    } else {

        //get timestamp
        time_t rawtime;
        struct tm *tm;
        time(&rawtime);
        tm = localtime(&rawtime);

        //append timestamped log line to file
        fprintf(fp, "[%i:%i:%i]: %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, log);

        if (fclose(fp) != 0)
        {
            fprintf(stderr, "write_log(): Error closing log file for peer "
                    "%d\n", peer);
        }
    }
}
