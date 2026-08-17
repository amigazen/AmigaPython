/*
 * Prototypes for host trampolines linked into the plugin (pyamiga_posix.c /
 * pyamiga_api.c). Included after PosixLib function-like macros are #undef'd
 * so calls are not treated as returning int (vbcc implicit declaration).
 */
#ifndef PYAMIGA_SOCKPROTO_H
#define PYAMIGA_SOCKPROTO_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

int socket(int domain, int type, int protocol);
int bind(int s, const struct sockaddr *addr, socklen_t len);
int listen(int s, int backlog);
int accept(int s, struct sockaddr *addr, socklen_t *len);
int connect(int s, const struct sockaddr *addr, socklen_t len);
int shutdown(int s, int how);
int close(int fd);
int closesocket(int s);
int select(int nfds, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tv);
int recv(int s, void *buf, size_t len, int flags);
int send(int s, const void *buf, size_t len, int flags);
int recvfrom(int s, void *buf, size_t len, int flags,
             struct sockaddr *addr, socklen_t *alen);
int sendto(int s, const void *buf, size_t len, int flags,
           const struct sockaddr *addr, socklen_t alen);
int setsockopt(int s, int level, int optname, const void *optval,
               socklen_t optlen);
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int getsockname(int s, struct sockaddr *addr, socklen_t *len);
int getpeername(int s, struct sockaddr *addr, socklen_t *len);
int gethostname(char *name, size_t len);
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);
struct protoent *getprotobyname(const char *name);
unsigned long inet_addr(const char *cp);
char *inet_ntoa(struct in_addr in);
unsigned short htons(unsigned short hostshort);
unsigned long htonl(unsigned long hostlong);
unsigned short ntohs(unsigned short netshort);
unsigned long ntohl(unsigned long netlong);
int ioctl(int fd, unsigned long request, char *arg);
int fcntl(int fd, int cmd, int arg);
int dup(int fd);
FILE *fdopen(int fd, const char *mode);
int fclose(FILE *fp);
char *strerror(int errnum);

#endif /* PYAMIGA_SOCKPROTO_H */
