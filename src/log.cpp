#include "log.h"

FILE* lf; // Log file

void initLog() { 
    lf = fopen("t42.log", "w+");
    logToFile("Start logging");
}

void logToFile(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(lf, fmt, args);
    fprintf(lf, "\n");
    fflush(lf);

    va_end(args);
}
