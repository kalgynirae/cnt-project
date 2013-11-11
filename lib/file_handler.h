#ifndef _file_handler_h
#define _file_handler_h

int file_split(char* FILENAME, int FS, int PS, int P_ID);
/* read a file with name FILENAME of size FS 
 * and split it into pieces of PS bytes saved under directory of
 * peer with ID P_ID
 */


int file_merge(char* FILENAME, int FS, int PS, int P_ID){
/* write a file with name FILENAME of size FS 
 * from pieces of size PS bytes saved under directory of
 * peer with ID P_ID
 */

#endif