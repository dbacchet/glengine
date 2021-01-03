#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>

// default: log level to info
#ifndef LOG_LEVEL
#define LOG_LEVEL 4
#endif

// default: use only the file name, not the full path
#ifndef LOG_FULL_FILENAME
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
#define __FILENAME__ __FILE__
#endif

#ifdef LOG_NOCOLOR
#define LOG_COLOR_RED
#define LOG_COLOR_GREEN
#define LOG_COLOR_YELLOW
#define LOG_COLOR_BLUE
#define LOG_COLOR_MAGENTA
#define LOG_COLOR_CYAN
#define LOG_COLOR_GRAY
#define LOG_COLOR_DEFAULT_FG
#define LOG_COLOR_RESET
#else
#define LOG_COLOR_RED        "\33[31m"
#define LOG_COLOR_GREEN      "\33[32m"
#define LOG_COLOR_YELLOW     "\33[33m"
#define LOG_COLOR_BLUE       "\33[34m"
#define LOG_COLOR_MAGENTA    "\33[35m"
#define LOG_COLOR_CYAN       "\33[36m"
#define LOG_COLOR_GRAY       "\33[90m"
#define LOG_COLOR_DEFAULT_FG "\33[39m"
#define LOG_COLOR_RESET      "\33[0m"
#endif

/* safe readable version of errno */
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_message(MSG, ...) printf(MSG "\n", ##__VA_ARGS__)
#define log_debug(MSG, ...)   printf(LOG_COLOR_MAGENTA "[DEBUG] " LOG_COLOR_RESET MSG "  " LOG_COLOR_GRAY " at %s (%s:%d) " LOG_COLOR_DEFAULT_FG "\n", ##__VA_ARGS__, __func__, __FILENAME__, __LINE__)
#define log_info(MSG, ...)    printf(LOG_COLOR_GREEN   "[INFO ] " LOG_COLOR_RESET MSG "\n", ##__VA_ARGS__)
#define log_warning(MSG, ...) printf(LOG_COLOR_YELLOW  "[WARN ] " LOG_COLOR_RESET MSG "  " LOG_COLOR_GRAY " at %s (%s:%d) " LOG_COLOR_DEFAULT_FG "\n", ##__VA_ARGS__, __func__, __FILENAME__, __LINE__)
#define log_error(MSG, ...)   printf(LOG_COLOR_RED     "[ERROR] " LOG_COLOR_RESET MSG "  " LOG_COLOR_GRAY " at %s (%s:%d) " LOG_COLOR_DEFAULT_FG "\n", ##__VA_ARGS__, __func__, __FILENAME__, __LINE__)

#if defined(NDEBUG) || LOG_LEVEL < 4
#undef log_debug
#define log_debug(MSG, ...)
#endif

#if LOG_LEVEL < 3
#undef log_info
#define log_info(MSG, ...)
#endif

#if LOG_LEVEL < 2
#undef log_warning
#define log_warning(MSG, ...)
#endif

#if LOG_LEVEL < 1
#undef log_error
#define log_error(MSG, ...)
#endif
