#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "core/error.h"
#include "scone_config.h"
#include "sys/sys.h"

int main(int argc, const char **argv) {
    SconeConfig_t cfg;
    cfg_error_t err;

    // Load configuration, PANIC if invalid (logs exact schema violation)
    if (SconeConfig_load(&cfg, "config/scone.ini", argc, argv, &err) != CFG_SUCCESS) {
        PANIC("Configuration error in '%s': %s", err.field, err.message);
    }

    size_t active_threads = cfg.threads;
    bool auto_cpu_cores = false;
    if (active_threads == 0) {
        auto_cpu_cores = true;
        active_threads = get_system_cpu_cores();
        LOG_INFO("Auto-detected %zu system cores", active_threads);
    }
    if(!auto_cpu_cores) LOG_INFO("Threads: %zu", active_threads);
    LOG_INFO("Cost (Max AST Depth): %d", (int)cfg.cost);
    
    // TODO: Add core application logic here

    SconeConfig_free(&cfg);
    return EXIT_SUCCESS;
}
