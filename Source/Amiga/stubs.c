/*
 *      stubs.c - common stubs for bsdsocket.library
 *
 *      Based on Irmen de Jong's original Amiga port
 *      Updated for Python 2.7.18
 */

#undef _OPTINLINE
#include <proto/socket.h>

/*
 * Select stub for compatibility with BSD sockets
 * Uses WaitSelect() with NULL signal mask
 */
int 
select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
       struct timeval *timeout)
{
  /* call WaitSelect with NULL signal mask pointer */
  return WaitSelect(nfds, readfds, writefds, exeptfds, timeout, NULL);
}

/*
 * Convert IP address to string representation
 */
char * 
inet_ntoa(struct in_addr addr) 
{
  return Inet_NtoA(addr.s_addr);
}

/*
 * Create an internet address from network and host numbers
 */
struct in_addr 
inet_makeaddr(int net, int host)
{
  struct in_addr addr;
  addr.s_addr = Inet_MakeAddr(net, host);
  return addr;
}

/*
 * Get the local network address part from an internet address
 */
unsigned long 
inet_lnaof(struct in_addr addr) 
{
  return Inet_LnaOf(addr.s_addr);
}

/*
 * Get the network number part from an internet address
 */
unsigned long   
inet_netof(struct in_addr addr)
{
  return Inet_NetOf(addr.s_addr);
} 