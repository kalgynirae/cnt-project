#ifndef _file_handler_h
#define _file_handler_h

int file_split(char* FILENAME, int FS, int PS, int P_ID);
/* read a file with name FILENAME of size FS 
 * and split it into pieces of PS bytes saved under directory of
 * peer with ID P_ID
 */

int file_merge(char* FILENAME, int FS, int PS, int P_ID);
/* write a file with name FILENAME of size FS 
 * from pieces of size PS bytes saved under directory of
 * peer with ID P_ID
 */

/* read the content from a local piece file into buffer
 * return size of content read or a negative number on error
 */
int read_piece(unsigned int piece_idx, char** buffer, int PS, int P_ID);

/* get the content from a piece by reading the local piece file
 * return amount written on success or negative number on error
 */
int write_piece(unsigned int piece_idx, char** buffer);

#endif
