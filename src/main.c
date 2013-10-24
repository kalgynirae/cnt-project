#include <stdio.h>
#include "init.h"

#define CFG_PATH "config/Common.cfg"

int main(int argc, char *argv[]) {
    struct common_cfg cfg;
   
    cfg = read_cfg(CFG_PATH);

    printf("NumberOfPreferredNeighbors = %d\n", cfg.n_preferred_neighbors);
    printf("UnchokingInterval = %d\n", cfg.unchoke_interval);
    printf("OptimisticUnchokingInterval = %d\n", cfg.optimistic_unchoke_interval);
    printf("FileName = %s\n", cfg.file_name);
    printf("FileSize = %d\n", cfg.file_size);
    printf("PieceSize = %d\n", cfg.piece_size);

    return 0;
}
