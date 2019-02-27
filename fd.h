#ifndef TSW_FD_H_
#define TSW_FD_H_

#include <stdlib.h>
#include <unistd.h>

void tswCo_setnonblock(int fd);
ssize_t tswCo_read(int fd, void *buf, size_t count);
ssize_t tswCo_write(int fd, const void *buf, size_t count);

#endif /* TSW_FD_H_ */
