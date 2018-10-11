#ifndef OEPCIE_TESTFUNC_H
#define OEPCIE_TESTFUNC_H

int oe_cobs_stuff(uint8_t *dst, const uint8_t *src, size_t size);
double randn(double mu, double sigma);

#ifdef _WIN32
// Windows does not have a usleep()
// https://www.c-plusplus.net/forum/topic/109539/usleep-unter-window
void usleep(__int64 usec);

// Windows does not have a getline()
size_t getline(char **lineptr, size_t *n, FILE *stream);
#else

#include <time.h>
typedef struct timespec timespec_t;
timespec_t timediff(timespec_t start, timespec_t end);

#endif

#endif // OEPCIE_TESTFUNC_H
