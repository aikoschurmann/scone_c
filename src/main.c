#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "error.h"
#include "scone_config.h"

int main(int argc, const char **argv) {
    SconeConfig_t cfg;
    cfg_error_t err;

    // Load configuration, PANIC if invalid (logs exact schema violation)
    if (SconeConfig_load(&cfg, "config/scone.ini", argc, argv, &err) != CFG_SUCCESS) {
        PANIC("Configuration error in '%s': %s", err.field, err.message);
    }

    LOG_INFO("Threads: %d", (int)cfg.threads);
    LOG_INFO("Cost (Max AST Depth): %d", (int)cfg.cost);
    
    // TODO: Add core application logic here

    SconeConfig_free(&cfg);
    return EXIT_SUCCESS;
}
