
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>


// #define log_trace(...) syslog(LOG_TRACE, __VA_ARGS__)
#define log_debug(...) syslog(LOG_DEBUG, __VA_ARGS__)
#define log_info(...)  syslog(LOG_INFO, __VA_ARGS__)
#define log_warn(...)  syslog(LOG_WARNING,  __VA_ARGS__)
#define log_error(...) syslog(LOG_ERR, __VA_ARGS__)
#define log_fatal(...) syslog(LOG_CRIT, __VA_ARGS__)

void init_log(int level, char * name);
void close_log();
#endif
