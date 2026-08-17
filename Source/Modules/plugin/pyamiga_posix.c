/*
 * Socket trampolines into the host psocket table (posix fd over AmiTCP
 * LVOs). AmiTLS uses fn_socket_native_fd. DNS still uses PosixLib.
 * Undef PosixLib __P* macros so these symbols are the ones socketmodule links.
 */
#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"

#undef socket
#undef bind
#undef listen
#undef accept
#undef connect
#undef shutdown
#undef close
#undef closesocket
#undef select
#undef recv
#undef send
#undef recvfrom
#undef sendto
#undef setsockopt
#undef getsockopt
#undef getsockname
#undef getpeername
#undef gethostname
#undef gethostbyname
#undef gethostbyaddr
#undef getservbyname
#undef getservbyport
#undef getprotobyname
#undef inet_addr
#undef inet_ntoa
#undef htons
#undef htonl
#undef ntohs
#undef ntohl
#undef ioctl
#undef fcntl
#undef dup
#undef fdopen
#undef fclose
#undef strerror

int
socket(int domain, int type, int protocol)
{
    return PyAmiga_Host->fn_socket(domain, type, protocol);
}

int
bind(int s, const struct sockaddr *addr, socklen_t len)
{
    return PyAmiga_Host->fn_bind(s, addr, len);
}

int
listen(int s, int backlog)
{
    return PyAmiga_Host->fn_listen(s, backlog);
}

int
accept(int s, struct sockaddr *addr, socklen_t *len)
{
    return PyAmiga_Host->fn_accept(s, addr, len);
}

int
connect(int s, const struct sockaddr *addr, socklen_t len)
{
    return PyAmiga_Host->fn_connect(s, addr, len);
}

int
shutdown(int s, int how)
{
    return PyAmiga_Host->fn_shutdown(s, how);
}

int
close(int fd)
{
    return PyAmiga_Host->fn_close(fd);
}

int
closesocket(int s)
{
    return PyAmiga_Host->fn_closesocket(s);
}

int
select(int nfds, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tv)
{
    return PyAmiga_Host->fn_select(nfds, rd, wr, ex, tv);
}

int
recv(int s, void *buf, size_t len, int flags)
{
    return PyAmiga_Host->fn_recv(s, buf, len, flags);
}

int
send(int s, const void *buf, size_t len, int flags)
{
    return PyAmiga_Host->fn_send(s, buf, len, flags);
}

int
recvfrom(int s, void *buf, size_t len, int flags,
         struct sockaddr *addr, socklen_t *alen)
{
    return PyAmiga_Host->fn_recvfrom(s, buf, len, flags, addr, alen);
}

int
sendto(int s, const void *buf, size_t len, int flags,
       const struct sockaddr *addr, socklen_t alen)
{
    return PyAmiga_Host->fn_sendto(s, buf, len, flags, addr, alen);
}

int
setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
    return PyAmiga_Host->fn_setsockopt(s, level, optname, optval, optlen);
}

int
getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
    return PyAmiga_Host->fn_getsockopt(s, level, optname, optval, optlen);
}

int
getsockname(int s, struct sockaddr *addr, socklen_t *len)
{
    return PyAmiga_Host->fn_getsockname(s, addr, len);
}

int
getpeername(int s, struct sockaddr *addr, socklen_t *len)
{
    return PyAmiga_Host->fn_getpeername(s, addr, len);
}

int
gethostname(char *name, size_t len)
{
    return PyAmiga_Host->fn_gethostname(name, len);
}

struct hostent *
gethostbyname(const char *name)
{
    return PyAmiga_Host->fn_gethostbyname(name);
}

struct hostent *
gethostbyaddr(const void *addr, socklen_t len, int type)
{
    return PyAmiga_Host->fn_gethostbyaddr(addr, len, type);
}

struct servent *
getservbyname(const char *name, const char *proto)
{
    return PyAmiga_Host->fn_getservbyname(name, proto);
}

struct servent *
getservbyport(int port, const char *proto)
{
    return PyAmiga_Host->fn_getservbyport(port, proto);
}

struct protoent *
getprotobyname(const char *name)
{
    return PyAmiga_Host->fn_getprotobyname(name);
}

unsigned long
inet_addr(const char *cp)
{
    return PyAmiga_Host->fn_inet_addr(cp);
}

char *
inet_ntoa(struct in_addr in)
{
    return PyAmiga_Host->fn_inet_ntoa(in);
}

unsigned short
htons(unsigned short hostshort)
{
    return PyAmiga_Host->fn_htons(hostshort);
}

unsigned long
htonl(unsigned long hostlong)
{
    return PyAmiga_Host->fn_htonl(hostlong);
}

unsigned short
ntohs(unsigned short netshort)
{
    return PyAmiga_Host->fn_ntohs(netshort);
}

unsigned long
ntohl(unsigned long netlong)
{
    return PyAmiga_Host->fn_ntohl(netlong);
}

int
ioctl(int fd, unsigned long request, char *arg)
{
    return PyAmiga_Host->fn_ioctl(fd, request, arg);
}

int
fcntl(int fd, int cmd, int arg)
{
    return PyAmiga_Host->fn_fcntl(fd, cmd, arg);
}

int
dup(int fd)
{
    return PyAmiga_Host->fn_dup(fd);
}

FILE *
fdopen(int fd, const char *mode)
{
    return PyAmiga_Host->fn_fdopen(fd, mode);
}

int
fclose(FILE *fp)
{
    return PyAmiga_Host->fn_fclose(fp);
}

char *
strerror(int errnum)
{
    return PyAmiga_Host->fn_strerror(errnum);
}
