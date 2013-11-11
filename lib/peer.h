#ifndef _peer_h
#define _peer_h

#include <time.h>
#include "init.h"
#include "receiver.h"

int peer_handle_data(struct peer_info *peer, char *data, int nbytes,
                     bitfield_t bitfield);
int peer_handle_timeout(struct peer_info *peer);

#endif
