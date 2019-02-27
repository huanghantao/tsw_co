#ifndef TSW_NET_H_
#define TSW_NET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include "coroutine.h"

int tswCo_accept(tswCo_schedule *S, int sockfd, struct sockaddr *addr, socklen_t *addrlen);

#endif /* TSW_NET_H_ */
