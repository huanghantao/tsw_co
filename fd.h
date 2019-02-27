#ifndef TSW_FD_H_
#define TSW_FD_H_

#include <stdlib.h>
#include <unistd.h>
#include "coroutine.h"

ssize_t tswCo_read(tswCo_schedule *S, int fd, void *buf, size_t count);
ssize_t tswCo_write(tswCo_schedule *S, int fd, const void *buf, size_t count);

#endif /* TSW_FD_H_ */
