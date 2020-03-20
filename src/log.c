#include <syslog.h>

void init_log(int level, char * name) {
    setlogmask(LOG_UPTO(level));
    openlog(name, LOG_CONS | LOG_PID| LOG_NDELAY , LOG_USER);
}

void close_log() {
    closelog();
}