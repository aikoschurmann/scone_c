#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "core/colors.h"

// Log Levels
#define LOG_LVL_DEBUG 0
#define LOG_LVL_INFO  1
#define LOG_LVL_WARN  2
#define LOG_LVL_ERROR 3

// Default verbosity if not specified during build (-DLOG_LEVEL=...)
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LVL_INFO
#endif

// Core Logging Macros
#define LOG_DEBUG(...) do { if (LOG_LEVEL <= LOG_LVL_DEBUG) { printf(C_GRAY "[DEBUG]" C_RST " "); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } } while(0)
#define LOG_INFO(...)  do { if (LOG_LEVEL <= LOG_LVL_INFO)  { printf(C_CYN "[INFO] " C_RST " "); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } } while(0)
#define LOG_WARN(...)  do { if (LOG_LEVEL <= LOG_LVL_WARN)  { fflush(stdout); fprintf(stderr, C_YEL "[WARN] " C_RST " "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } } while(0)
#define LOG_ERR(...)   do { if (LOG_LEVEL <= LOG_LVL_ERROR) { fflush(stdout); fprintf(stderr, C_RED "[ERROR] " C_GRAY "%s:%d: " C_RST, __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } } while(0)

// Fatal panic that automatically crashes
#define PANIC(...) do { LOG_ERR(__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

// BUBBLE_UP: A clean way to catch errors, add context, and return up the call stack
#define BUBBLE_UP(cond, ret_val, ...) \
    do { \
        if (!(cond)) { \
            LOG_ERR(__VA_ARGS__); \
            return (ret_val); \
        } \
    } while(0)
