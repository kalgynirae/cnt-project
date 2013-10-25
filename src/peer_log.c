#include "peer_log.h"

/**
 * Functions used for logging events in the process
 */

//a connection was established between 2 peers, initiated by peer #from_peer
void log_connect(int from_peer, int to_peer)
{
}

//a peer has changed its preferred neighbors
void log_change_preferred(int peer, int new_preferred[]);

//a peer has optimistically unchoked unchoked_peer
void log_optimistic_unchoke(int peer, int unchoked_peer);

//reciever got UNCHOKE message from sender
void log_unchoked_by(int reciever, int sender);

//reciever got CHOKE message from sender
void log_recieve_choke(int reciever, int sender);

//reciever got HAVE message from sender
void log_recieved_have(int reciever, int sender);

//reciever got INTERESTED message from sender
void log_recieved_interested(int reciever, int sender);

//reciever got NOT_INTERESTED message from sender
void log_recieved_not_interested(int reciever, int sender);

//peer has finished downloading piece
void log_downloaded_piece(int peer, int piece);

//peer has finished all pieces of file
void log_downloaded_file(int peer);

//write a log to the corresponding peer log file
void write_log(int peer, char* log)
{
    FILE* fp;
    char filename[LOG_FILENAME_LEN];
    snprintf(filename, LOG_FILENAME_LEN + 1, LOG_FILENAME, peer);

    //open in append mode
    if ((fp = fopen(filename, "a")) == NULL) {
        printf("Error opening log file for peer %d", peer);
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
            printf("Error closing log file for peer %d", peer);
        }
    }
}
