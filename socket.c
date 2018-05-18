#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "socket.h"

int serv_listen (const char* name)
{
    int fd,len;
    struct sockaddr_un unix_addr;

    /* Create a Unix domain stream socket */
    if ( (fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
        return (-1);

    fcntl (fd, F_SETFD, FD_CLOEXEC);
    unlink (name);

    /* Fill in socket address structure */
    memset (&unix_addr, 0, sizeof (unix_addr));
    unix_addr.sun_family = AF_UNIX;
    strcpy (unix_addr.sun_path, name);
    len = sizeof (unix_addr.sun_family) + strlen (unix_addr.sun_path);

    /* Bind the name to the descriptor */
    if (bind (fd, (struct sockaddr*)&unix_addr, len) < 0) {
        goto error;
    }
    if (chmod (name, 0666) < 0) {
        goto error;
    }

    if (listen (fd, 5) < 0) {
        goto error;
    }
    return (fd);
   
error:
    close (fd);
    return (-1); 
}




#define    STALE    30    /* client's name can't be older than this (sec) */

/* Wait for a client connection to arrive, and accept it.
 * We also obtain the client's pid from the pathname
 * that it must bind before calling us. */

/* returns new fd if all OK, < 0 on error */
int serv_accept (int listenfd, pid_t *pidptr) //, uid_t *uidptr)
{
    int                clifd, len;
    time_t             staletime;
    struct sockaddr_un unix_addr;
    struct stat        statbuf;
    const char*        pid_str;

    len = sizeof (unix_addr);
    if ( (clifd = accept (listenfd, (struct sockaddr *) &unix_addr, &len)) < 0)
        return (-1);        /* often errno=EINTR, if signal caught */

    /* obtain the client's uid from its calling address */
    len -= /* th sizeof(unix_addr.sun_len) - */ sizeof(unix_addr.sun_family);
                    /* len of pathname */
    unix_addr.sun_path[len] = 0;            /* null terminate */
    if (stat(unix_addr.sun_path, &statbuf) < 0)
        return(-2);
#ifdef    S_ISSOCK    /* not defined for SVR4 */
    if (S_ISSOCK(statbuf.st_mode) == 0)
        return(-3);        /* not a socket */
#endif
    if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) ||
        (statbuf.st_mode & S_IRWXU) != S_IRWXU)
          return(-4);    /* is not rwx------ */

    staletime = time(NULL) - STALE;
    if (statbuf.st_atime < staletime ||
        statbuf.st_ctime < staletime ||
        statbuf.st_mtime < staletime)
          return(-5);    /* i-node is too old */

//    if (uidptr != NULL)
//        *uidptr = statbuf.st_uid;    /* return uid of caller */

    /* get pid of client from sun_path */
    pid_str = strrchr (unix_addr.sun_path, '/');
    pid_str++;

    *pidptr = atoi (pid_str);
    
    unlink (unix_addr.sun_path);        /* we're done with pathname now */
    return (clifd);
}


#define CLI_PATH    "/var/tmp/"        /* +5 for pid = 14 chars */
#define CLI_PERM    S_IRWXU            /* rwx for user only */

/* returns fd if all OK, -1 on error */
int cli_conn (const char *name, char project)
{
    int                fd, len;
    struct sockaddr_un unix_addr;

    /* create a Unix domain stream socket */
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return(-1);

    /* fill socket address structure w/our address */
    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    sprintf(unix_addr.sun_path, "%s%05d-%c", CLI_PATH, getpid(), project);
    len = /* th sizeof(unix_addr.sun_len) + */ sizeof(unix_addr.sun_family) +
          strlen(unix_addr.sun_path) /* th + 1 */;
    /* th unix_addr.sun_len = len; */

    unlink (unix_addr.sun_path);        /* in case it already exists */
    if (bind(fd, (struct sockaddr *) &unix_addr, len) < 0)
        goto error;
    if (chmod(unix_addr.sun_path, CLI_PERM) < 0)
        goto error;

    /* fill socket address structure w/server's addr */
    memset(&unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    strcpy(unix_addr.sun_path, name);
    len = /* th sizeof(unix_addr.sun_len) + */ sizeof(unix_addr.sun_family) +
          strlen(unix_addr.sun_path) /* th + 1*/ ;
    /* th unix_addr.sun_len = len; */

    if (connect (fd, (struct sockaddr *) &unix_addr, len) < 0)
        goto error;

    return (fd);

error:
    close (fd);
    return(-1);
}

int sock_read (int fd, void* buff, int count)
{
    void* pts = buff;
    int status = 0, n;

    if (count <= 0) return SOCKERR_OK;

    while (status != count) {
        n = read (fd, pts + status, count - status);

        if (n < 0) { 
            if (errno == EINTR) {
                continue;
            }
            else
                return SOCKERR_IO;
        }

        if (n == 0)
            return SOCKERR_CLOSED;

        status += n;
    }

    return status;
}

int sock_write (int fd, const void* buff, int count)
{
    const void* pts = buff;
    int status = 0, n;

    if (count < 0) return SOCKERR_OK;

    while (status != count) {
        n = write (fd, pts + status, count - status);
        if (n < 0) {
            if (errno == EPIPE)
                return SOCKERR_CLOSED;
            else if (errno == EINTR) {
                continue;
            }
            else
                return SOCKERR_IO;
        }
        status += n;
    }

    return status;
}

