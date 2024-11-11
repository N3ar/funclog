#include <stdlib.h>
#include <logger.h>

int main() {
    printf("Testing c-logger\n");

    //arg1 filename, arg2 maxFileSize, arg3 maxBackupFiles
    logger_initFileLogger("logtest.txt", 1024, 2);
    logger_setLevel(LogLevel_DEBUG);

    LOG_INFO("file logging");
    LOG_DEBUG("format example: %d%c%s", 1, '2', "3");
    logger_log(LogLevel_INFO, __FILENAME__, 0, "Random Message test");

    return 0;
}
