#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <errno.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>

#include <limits.h>        /* for OPEN_MAX */

#include "server_config.h"

#define MAXLINE 80
#define SBUF_SIZE 100

sig_atomic_t is_stop = 0;

void skeleton_daemon();
void read_config_file(const char *configFilename);
void signal_handler(int sig);

int main(int argc, char *argv[])
{
    skeleton_daemon();

    read_config_file(CONFIG_FILE_PATH);

    struct pollfd           client[OPEN_MAX];
    struct sockaddr_in      servaddr;

   int listenfd = socket(AF_INET, SOCK_STREAM, 0);
   if(listenfd < 0){
       syslog(LOG_NOTICE, PROJECT_NAME" socket errno: %d", errno);
       goto exit;
   }

   memset(&servaddr, 0, sizeof(servaddr));

   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
   servaddr.sin_port        = htons( SERV_PORT );
   int res = bind(listenfd , (struct sockaddr *)&servaddr , sizeof( servaddr ));
   if(res < 0){
       syslog(LOG_NOTICE, PROJECT_NAME" bind errno: %d", errno);
       goto exit;
   }

   res = listen(listenfd, 1024);
   if(res < 0){
       syslog(LOG_NOTICE, PROJECT_NAME" listen errno: %d", errno);
       goto exit;
   }

   client[0].fd = listenfd;
   client[0].events = POLLRDNORM;
   int i, maxi , connfd , sockfd;
   int nready;

   for( i = 1 ; i < OPEN_MAX ; i ++ ){
       client[i].fd = -1;
   }
   maxi = 0;

   while (!is_stop) {
        nready = poll(client, maxi + 1 , -1);
        if(client[0].revents & POLLRDNORM)
        {
            struct sockaddr_in cliaddr;
            int clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr , &clilen);

            //write(connfd, "CENTRE|NEW_JOB|1|sword|A|C|CENTRE\0", 255);
            for( i = 1 ; i < OPEN_MAX ; i ++ ){
                if( client[i].fd < 0 ){
                    client[i].fd = connfd;
                    break;
                }
            }

            if(i == OPEN_MAX){
                syslog(LOG_NOTICE, PROJECT_NAME" too many clients!");
                goto exit;
            }

            client[i].events = POLLRDNORM;
            if(i > maxi){
                maxi = i;
            }

            if(--nready <= 0){
                continue;
            }
        }

        for( i = 1 ; i <= maxi ; i ++ )
        {
            if( ( sockfd = client[i].fd ) < 0 )
                    continue;

            if( client[i].revents & ( POLLRDNORM | POLLERR ) )
            {
                char buf[MAXLINE] = {0};
                ssize_t n = 0;
                if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        syslog(LOG_NOTICE, PROJECT_NAME" client[%d] aborted connection", i);
                        close(sockfd);
                        client[i].fd = -1;
                    }
                    else{
                        syslog(LOG_NOTICE, PROJECT_NAME" read error");
                    }
                }
                else if (n == 0) {
                    syslog(LOG_NOTICE, PROJECT_NAME" client[%d] closed connection", i);

                    close(sockfd);
                    client[i].fd = -1;
                }
                else{
                    if(strncmp(buf, GET_VERSION, strlen(GET_VERSION)) == 0){
                        if(curLen){
                            write(sockfd, version, curLen);
                        }
                    }

                    close( sockfd );
                    client[i].fd = -1;
                }

            if( -- nready <= 0 )
                break;
            }
        }
    }

exit:
    syslog(LOG_NOTICE, PROJECT_NAME" terminated.");
    closelog();

    return EXIT_SUCCESS;
}


void read_config_file(const char *configFilename)
{
    FILE *configfp = fopen(configFilename, "r");
    if (configfp != NULL) {                 /* Ignore nonexistent file */
        while (!feof(fp))
        {
            if (fgets(version, SBUF_SIZE, configfp) != NULL){
                curLen = strlen(version);
                version[curLen] = '\0';    /* Strip trailing '\n' */
                syslog(LOG_NOTICE, PROJECT_NAME" read version file version is: %s", version);
            }

        }

        fclose(configfp);
    }
}

void signal_handler(int sig)
{
    if(sig == SIGHUP){
        read_config_file(CONFIG_FILE_PATH);
    }
    else if(sig == SIGINT){
        is_stop = 1;
    }
}

void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog(PROJECT_NAME, LOG_PID, LOG_DAEMON);
}
