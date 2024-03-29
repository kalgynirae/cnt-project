#ifndef _peer_log_h
#define _peer_log_h

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

/**
 * Functions used for logging events between peers
 */

//peer ids are 4 digits
#define LOG_FILENAME "runtime/peer_%4d.log"
//length of log filename. remove %4d (-3) but add 4 digits, so +1
#define LOG_FILENAME_LEN sizeof(LOG_FILENAME) + 1

//a connection was established between 2 peers, initiated by peer #from_peer
void log_connect(int from_peer, int to_peer);

//a peer has changed its preferred neighbors
void log_change_preferred(int peer, int num_preferred, int new_preferred[]);

//a peer has optimistically unchoked unchoked_peer
void log_optimistic_unchoke(int peer, int unchoked_peer);

//receiver got UNCHOKE message from sender
void log_unchoked_by(int receiver, int sender);

//receiver got CHOKE message from sender
void log_receive_choke(int receiver, int sender);

//receiver got HAVE message from sender
void log_received_have(int receiver, int sender, int piece_idx);

//receiver got INTERESTED message from sender
void log_received_interested(int receiver, int sender);

//receiver got NOT_INTERESTED message from sender
void log_received_not_interested(int receiver, int sender);

//peer has finished downloading piece
void log_downloaded_piece(int peer, int piece, int other_peer, int num_pieces);

//peer has finished all pieces of file
void log_downloaded_file(int peer);

//write a log to the corresponding peer log file
void write_log(int peer, char* log);

#endif
