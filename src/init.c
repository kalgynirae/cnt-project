#include "init.h"

//global configuration options
struct common_cfg g_config;
int g_bitfield_len;

void read_cfg(char* cfg_file_name)
{
    FILE *fp;               //file pointer
    char *line = NULL;      //current line read
    size_t len = 0;         
    ssize_t read;

    if ((fp = fopen(cfg_file_name, "r")) == NULL) {
        printf("Error opening file");
    } else {
        while ((read = getline(&line, &len, fp)) != -1) {
            parse_cfg_line(line, &g_config);
        }

        if (line)
            free(line);

        if (fclose(fp) != 0)
        {
            printf("Error closing file");
        }
    }
    g_bitfield_len = g_config.file_size / g_config.piece_size;
    //add 1 if not an even division to accomodate last partial piece
    g_bitfield_len += (g_config.file_size % g_config.piece_size == 0) ? 0 : 1;
}

struct peer_info* read_peers(char* cfg_file_name, int *num_peers,
                             int our_peer_id)
{
    struct peer_info *peers;
    int peer_count = 0;        //number of peers in file
    FILE *fp;                 //file pointer
    char *line = NULL;        //current line read
    size_t len = 0;         
    ssize_t read;

    if ((fp = fopen(cfg_file_name, "r")) == NULL) {
        fprintf(stderr, "read_peers(): Error opening file\n");
    } else {

        while ((read = getline(&line, &len, fp)) != -1) {
            peer_count++;    //count number of peers
        }
        rewind(fp);         //start from beginning to read entries
        peer_count--;       //but we don't count ourself as a peer
        *num_peers = peer_count;

        //create peer[] of proper size
        peers = (struct peer_info*)(malloc(sizeof(struct peer_info) * peer_count));
        int i = 0;
        struct peer_info temp_info;

        while ((read = getline(&line, &len, fp)) != -1) {
            temp_info = parse_peer_line(line);
            if (temp_info.peer_id != our_peer_id)
            {
                /* If we get to the last line of the file and we haven't found
                 * our_peer_id yet (if we had, the counter would be one less),
                 * then error!
                 */
                if (i == *num_peers)
                {
                    fprintf(stderr, "read_peers(): launched with peer_id=%d "
                                    "but no such peer found in file\n",
                            our_peer_id);
                    return NULL;
                }
                peers[i] = temp_info;
                i++;
            }
        }

        if (line)
            free(line);

        if (fclose(fp) != 0)
        {
            fprintf(stderr, "read_peers(): Error closing file\n");
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
