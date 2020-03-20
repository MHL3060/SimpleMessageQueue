#include <syslog.h>

init_log(int level, char * name) {
    setlogmask(LOG_UPTO(level));
    openlog(name, LOG_CONS | LOG_PID| LOG_NDELAY , LOG_USER);
}

close_log() {
    closelog();
}