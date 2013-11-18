#include <stdio.h>
#include <stdlib.h>
#include "file_handler.h"

int file_split(char* FILENAME, int FS, int PS, int P_ID){
	//FILENAME: cfg.file_name
	//FS: cfg.file_size
	//PS: cfg.piece_size
	//P_ID: peer ID

	FILE *fpr;//file pointer for reading file
	FILE *fpw;//file pointer for writing to file
	char c;//character read from file
	char FILEWRITE [20];//name of file piece to be written
	int i, j;
	fpr = fopen(FILENAME,"r");
	if(fpr == NULL){
		printf("File does not exist\n");
	}else{
		for(j = 1; j <= (FS/PS); j++){//read each piece of file
			sprintf(FILEWRITE, "runtime/peer_%d/piece_%d", P_ID, j);
			fpw = fopen(FILEWRITE,"w");
			for(i = 1; i <= PS; i++){//copy piece of file byte by byte
				c = getc(fpr);//read bye of file
				if(c != EOF){//copy file byte
					fprintf(fpw, "%c", c);
				}else{//exit loop if end of file is reached
					break;
				}
			}
			fclose(fpw);
		}
		fclose(fpr);
	}
    return 0;
}

int file_merge(char* FILENAME, int FS, int PS, int P_ID){
	//FILENAME: cfg.file_name
	//FS: cfg.file_size
	//PS: cfg.piece_size
	//P_ID: peer ID

	FILE *fpr;//file pointer for reading file
	FILE *fpw;//file pointer for writing to file
	char c;//character read from file
	char FILEREAD [20];//name of file piece to be read
	int i, j;
	fpw = fopen(FILENAME,"w");
	for(j = 1; j <= (FS/PS); j++){//write each piece of file
		sprintf(FILEREAD, "\\peer_%d\\piece_%d", P_ID, j);
		fpr = fopen(FILEREAD,"r");
		if(fpr == NULL){
			printf("ERROR: file piece %d does not exist\n", j);
			break;
		}else{
			for(i = 1; i <= PS; i++){//copy piece of file byte by byte
				c = getc(fpr);//read bye of file
				if(c != EOF){//copy file byte
					fprintf(fpw, "%c", c);
				}else{//exit loop if end of file is reached
					break;
				}
			}
		}
		fclose(fpr);
	}
	fclose(fpw);
    return 0;
}

//read the content from a local piece file into buffer
//return size of content read or a negative number on error
int read_piece(unsigned int piece_idx, char** buffer, int PS, int P_ID)
{
    //convert the piece index into the corresponding file path
	FILE *fpr;//file pointer for reading file
	char c;//character read from file
	int FS = 0;//counter to be returned telling the file size
	*buffer = malloc(PS);//allocate space for buffer equal to the piece size
	char FILEREAD [20];//name of file piece to be read
	sprintf(FILEREAD, "\\peer_%d\\piece_%d", P_ID, piece_idx);
	fpr = fopen(FILEREAD,"r");
	if(fpr == NULL){
		printf("ERROR: file piece %d does not exist\n", piece_idx);
		FS = -1;
	}else{
		int i;
		//read the file into buffer
		for(i = 0; i < PS; i++){
			c = getc(fpr);//read bye of piece
			if(c != EOF){
				*buffer[i] = c;
				FS++;
			}else{//exit loop if end of file is reached
				break;
			}
		}
	}
	fclose(fpr);
    //return the size of the content read
    return FS;
}

/* write the content of a piece to the local piece file
 * len is the length of the content contained in buffer
 * return amount written on success or negative number on error
 */
int write_piece(unsigned int piece_idx, unsigned int len, char* buffer)
{
    //convert the piece index into the corresponding file path
    //error if file already exists
    //create file and write content to file
    return 0;
}
