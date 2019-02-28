#ifndef TSW_NET_H_
#define TSW_NET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "coroutine.h"

int tswCo_accept(tswCo_schedule *S, int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t tswCo_recv(tswCo_schedule *S, int connfd, void *buf, size_t len, int flags);
ssize_t tswCo_send(tswCo_schedule *S, int connfd, void *buf, size_t len, int flags);
int tswCo_shutdown(tswCo_schedule *S, int socket, int how);

#endif /* TSW_NET_H_ */
