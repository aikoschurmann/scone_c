#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "scone_config.h"

int main(int argc, const char **argv) {
    SconeConfig_t cfg;
    cfg_error_t err;

    if (SconeConfig_load(&cfg, "config/scone.ini", argc, argv, &err) != CFG_SUCCESS) {
        fprintf(stderr, "FATAL: Configuration error in '%s': %s\n", err.field, err.message);
        return EXIT_FAILURE;
    }

    printf("Starting Scone...\n");
    printf(" - Threads: %d\n", (int)cfg.threads);
    printf(" - Cost (Max AST Depth): %d\n", (int)cfg.cost);
    
    // TODO: Add core application logic here

    SconeConfig_free(&cfg);
    return EXIT_SUCCESS;
}
