#include "sys.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <unistd.h>
#endif

size_t get_system_cpu_cores(void) {
#if defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (size_t)sysinfo.dwNumberOfProcessors;
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores > 0) {
        return (size_t)cores;
    }
    return 1; // Fallback
#else
    return 1; // Fallback
#endif
}
