#include "init.h"

struct common_cfg read_cfg(char* cfg_file_name)
{
    struct common_cfg cfg;  //config struct to populate and return
    FILE *fp;               //file pointer
    char *line = NULL;      //current line read
    size_t len = 0;         
    ssize_t read;

    if ((fp = fopen(cfg_file_name, "r")) == NULL) {
        printf("Error opening file");
    } else {
        while ((read = getline(&line, &len, fp)) != -1) {
            parse_cfg_line(line, &cfg);
        }

        if (line)
            free(line);

        if (fclose(fp) != 0)
        {
            printf("Error closing file");
        }
    }

    return cfg;
}

void parse_cfg_line(char *line, struct common_cfg *cfg)
{
    char *key = strtok(line, " ");  //part before space
    char *val = strtok(NULL, " ");  //part after space (null uses previous str)
    printf(key);
    printf("-");
    printf(val);

    //check key
    if (strcmp(key, "NumberOfPreferredNeighbors") != 0) {
        cfg->n_preferred_neighbors = atoi(val);
    }
    else if (strcmp(key, "UnchokingInterval") != 0) {
        cfg->unchoke_interval = atoi(val);
    }
    else if (strcmp(key, "OptimisticUnchokingInterval") != 0) {
        cfg->optimistic_unchoke_interval = atoi(val);
    }
    else if (strcmp(key, "FileName") != 0) {
        cfg->file_name  = val;
    }
    else if (strcmp(key, "FileSize") != 0) {
        cfg->file_size = atoi(val);
    }
    else if (strcmp(key, "PieceSize") != 0) {
        cfg->piece_size = atoi(val);
    }
    else {
        printf("%s is invalid as a config entry", val);
    }

}
