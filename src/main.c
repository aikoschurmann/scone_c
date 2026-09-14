#include <stdio.h>
#include <stdbool.h>

#define CONFIG_IMPLEMENTATION
#include "app.h"

int main(int argc, const char **argv) {
    App_t cfg;
    cfg_error_t err;

    // Load from config/scone.ini, environment variables, and CLI arguments
    if (App_load(&cfg, "config/scone.ini", argc, argv, &err) == CFG_SUCCESS) {
        printf("Configuration loaded successfully!\n");
        printf("Message: %s\n", cfg.message);
        printf("Port: %d\n", (int)cfg.port);
        App_free(&cfg);
    } else {
        fprintf(stderr, "Configuration error in '%s': %s\n", err.field, err.message);
        return 1;
    }

    return 0;
}
