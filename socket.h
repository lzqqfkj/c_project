
#ifndef _SOCKET_H
#define _SOCKET_H

#include <sys/types.h>
#include <unistd.h>

#define CS_PATH     "/var/tmp/minigui"

#define SOCKERR_IO          -1
#define SOCKERR_CLOSED      -2
#define SOCKERR_INVARG      -3
#define SOCKERR_TIMEOUT     -4
#define SOCKERR_OK          0

int serv_listen (const char* name);
int serv_accept (int listenfd, pid_t *pidptr); //, uid_t *uidptr);

int cli_conn (const char *name, char project);

int sock_read (int fd, void* buff, int count);
int sock_write (int fd, const void* buff, int count);

#endif /* end of _SOCKET_H */
