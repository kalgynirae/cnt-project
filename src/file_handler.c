
#include <stdio.h>

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
			sprintf(FILEWRITE, "\\peer_%d\\piece_%d", P_ID, j);
			fpw = fopen(FILEWRITE,"w");
			for(i = 1; i <= PS; i++){//copy piece of file byte by byte
				c = getc(fpr);//read bye of file
				if(c != EOF){//copy file byte
					fprintf(fpw, "%s", c);
				}else{//exit loop if end of file is reached
					break;
				}
			}
			fclose(fpw);
		}
		fclose(fpr);
	}

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
					fprintf(fpw, "%s", c);
				}else{//exit loop if end of file is reached
					break;
				}
			}
		}
		fclose(fpr);
	}
	fclose(fpw);
	

}