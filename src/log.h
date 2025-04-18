#include <stdarg.h>
#include <stdio.h>

#define LOG logToFile

// #define LOG(...)                                                               \
//     fprintf(lf, __VA_ARGS__);                                                  \
//     fprintf(lf, "\n");                                                         \
//     fflush(lf)

void initLog();
void logToFile(const char* fmt, ...);
