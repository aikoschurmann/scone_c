#include <stdio.h>
#include <stdbool.h>

#define CONFIG_IMPLEMENTATION
#include "scone_config.h"

int main(int argc, const char **argv) {
    SconeConfig_t cfg;
    cfg_error_t err;

    // Load from config/scone.ini, environment variables, and CLI arguments
    if (SconeConfig_load(&cfg, "config/scone.ini", argc, argv, &err) == CFG_SUCCESS) {
        printf("Configuration loaded successfully!\n");
        printf("Cost (Max AST Depth): %d\n", (int)cfg.cost);
        printf("Threads: %d\n", (int)cfg.threads);
        SconeConfig_free(&cfg);
    } else {
        fprintf(stderr, "Configuration error in '%s': %s\n", err.field, err.message);
        return 1;
    }

    return 0;
}
