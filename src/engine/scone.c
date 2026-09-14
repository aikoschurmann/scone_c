#include "engine/scone.h"
#include "core/error.h"
#include "scone_config.h"
#include "sys/sys.h"

#include <stdlib.h>

static size_t resolve_thread_count(size_t config_threads) {
    if (config_threads == 0) {
        size_t auto_cores = get_system_cpu_cores();
        LOG_INFO("Auto-detected %zu system cores", auto_cores);
        return auto_cores;
    }
    LOG_INFO("Threads: %zu", config_threads);
    return config_threads;
}

int scone_main(int argc, const char **argv) {
    SconeConfig_t cfg;
    cfg_error_t err;

    if (SconeConfig_load(&cfg, "config/scone.ini", argc, argv, &err) != CFG_SUCCESS) {
        PANIC("Configuration error in '%s': %s", err.field, err.message);
    }

    size_t active_threads = resolve_thread_count(cfg.threads);
    (void)active_threads; // Will be used by the engine later
    LOG_INFO("Cost (Max AST Depth): %d", (int)cfg.cost);
    
    // TODO: Initialize engine and start CEGIS loop

    SconeConfig_free(&cfg);
    return EXIT_SUCCESS;
}
