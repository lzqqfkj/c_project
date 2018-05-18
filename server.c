#include <sys/types.h>
#include <stdio.h>

#include "socket.h"

int main ()
{
    int listenfd = -1;
    int clifd;
    pid_t pid;
//    uid_t uid;
    int nread;
    int data;

    listenfd = serv_listen (CS_PATH);
    if (listenfd < 0) {
        printf ("serv_listen error.\n");
        return -1;
    }

    printf ("succeed to create socket and listen.\n");

    clifd = serv_accept (listenfd, &pid); //, &uid);
    if (clifd < 0) {
        printf ("serv_accept error. \n");
        return -1;
    }
    printf ("accept succeed to return. \n");
   
    while (1) { 
        printf ("Begin to read data... \n");

        nread = sock_read (clifd, &data, sizeof (int));
        if (nread == SOCKERR_IO) {
            printf ("read error on fd %d\n", clifd);
            break;
        } else if (nread == SOCKERR_CLOSED) {
            printf ("fd %d has been closed.\n", clifd);
            break;
        } else {
            printf ("Got that! data is %d\n", data);
        }
        nread = sock_write (clifd, &data, sizeof (int));

        if (nread == SOCKERR_IO) {
            printf ("write error on fd %d\n", clifd);
            break;
        } else if (nread == SOCKERR_CLOSED) {
            printf ("fd %d has been closed.\n", clifd);
            break;
        } else {
            printf ("Wrote %d to client. \n", data);
        }

        if (data == 1000) {
            break;
        }
        //sleep (10);
    }
        
    close (clifd);
    close (listenfd); 
    
    return 0; 
    
}
