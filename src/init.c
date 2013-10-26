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

struct peer_info* read_peers(char* cfg_file_name, int *num_peers)
{
    struct peer_info *peers;
    int peer_count = 0;        //number of peers in file
    FILE *fp;                 //file pointer
    char *line = NULL;        //current line read
    size_t len = 0;         
    ssize_t read;

    if ((fp = fopen(cfg_file_name, "r")) == NULL) {
        printf("Error opening file");
    } else {

        while ((read = getline(&line, &len, fp)) != -1) {
            peer_count++;    //count number of peers
        }
        rewind(fp);         //start from beginning to read entries
        *num_peers = peer_count;

        //create peer[] of proper size
        peers = (struct peer_info*)(malloc(sizeof(struct peer_info) * peer_count));
        int i = 0;

        while ((read = getline(&line, &len, fp)) != -1) {
            peers[i] = parse_peer_line(line);
            i++;
        }

        if (line)
            free(line);

        if (fclose(fp) != 0)
        {
            printf("Error closing file");
        }
    }

    return peers;
}

void parse_cfg_line(char *line, struct common_cfg *cfg)
{
    char *key = strtok(line, " ");  //part before space
    char *val = strtok(NULL, " ");  //part after space (null uses previous str)

    //check key
    if (strcmp(key, "NumberOfPreferredNeighbors") == 0) {
        cfg->n_preferred_neighbors = atoi(val);
    }
    else if (strcmp(key, "UnchokingInterval") == 0) {
        cfg->unchoke_interval = atoi(val);
    }
    else if (strcmp(key, "OptimisticUnchokingInterval") == 0) {
        cfg->optimistic_unchoke_interval = atoi(val);
    }
    else if (strcmp(key, "FileName") == 0) {
        cfg->file_name  = malloc(strlen(val) + 1);
        strcpy(cfg->file_name, val);
    }
    else if (strcmp(key, "FileSize") == 0) {
        cfg->file_size = atoi(val);
    }
    else if (strcmp(key, "PieceSize") == 0) {
        cfg->piece_size = atoi(val);
    }
    else {
        printf("%s is invalid as a config entry", val);
    }
}

struct peer_info parse_peer_line(char *line)
{
    struct peer_info info;

    info.peer_id = atoi(strtok(line, " "));        

    char* hostname = (strtok(NULL, " "));
    info.hostname = malloc(strlen(hostname) + 1);
    strcpy(info.hostname, hostname);

    info.port = atoi(strtok(NULL, " "));
    info.has_file = atoi(strtok(NULL, " "));

    info.state = 0;
    info.socket_fd = -1;

    return info;
}
