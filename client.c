
#include <stdio.h>
#include "socket.h"

int main ()
{
    int conn_fd = -1;
    int n = 0;
    int i = 0;
    int data = 0;

    conn_fd = cli_conn (CS_PATH, 'a');
    if (conn_fd < 0) {
        printf ("cli_conn error.\n");
        return -1;
    }
    
    for (i = 0; i < 1001; i++) {
        printf ("i = %d\n", i);
        n = sock_write (conn_fd, &i, sizeof (int));

        if (n == SOCKERR_IO) {
            printf ("write error on fd %d\n", conn_fd);
            break;
        } else if (n == SOCKERR_CLOSED) {
            printf ("fd %d has been closed.\n", conn_fd);
            break;
        } else {
            printf ("Wrote %d to server. \n", i);
        }

        n = sock_read (conn_fd, &data, sizeof (int));
        if (n == SOCKERR_IO) {
            printf ("read error on fd %d\n", conn_fd);
            break;
        } else if (n == SOCKERR_CLOSED) {
            printf ("fd %d has been closed.\n", conn_fd);
            break;
        } else {
            printf ("Got that! data is %d\n", data);
        }

        sleep (2);
    }

    return 0; 
    
}
