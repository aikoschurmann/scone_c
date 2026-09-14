#pragma once
#include <stddef.h>

/**
 * @brief Retrieves the number of logical CPU cores on the system.
 * 
 * @return The number of logical cores, or 1 if it cannot be determined.
 */
size_t get_system_cpu_cores(void);
