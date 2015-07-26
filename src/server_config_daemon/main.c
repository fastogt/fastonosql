#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>

#include <errno.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>

#include "uthash.h"

#include "server_config.h"

#define MAXLINE 80
#define SBUF_SIZE 256
#define SAVE_FREE(x) if(x) { free(x); x = NULL; }

sig_atomic_t is_stop = 0;

void skeleton_daemon();
void read_config_file(const char *configFilename);
void signal_handler(int sig);

struct setting {
    char *key;          /* key */
    char *value;          /* value */
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct setting * alloc_setting(const char *key, const char *value)
{
    struct setting * st = (struct setting*)malloc(sizeof(struct setting));
    if(!st){
        return NULL;
    }

    st->key = strdup(key);
    st->value = strdup(value);

    return st;
}

void free_setting(struct setting *st)
{
    if(!st){
        return;
    }

    SAVE_FREE(st->key);
    SAVE_FREE(st->value);
    SAVE_FREE(st);
}

struct setting *settings = NULL;

void add_setting(const char *key, const char *value)
{
    struct setting *s = NULL;

    HASH_FIND_STR(settings, key, s);  /* key already in the hash? */
    if (s == NULL) {
        struct setting * s = alloc_setting(key, value);
        HASH_ADD_STR(settings, key, s);  /* key: value of key field */
    }
    else{
        SAVE_FREE(s->value);
        s->value = strdup(value);
    }
}

struct setting *find_setting(const char *key)
{
    struct setting *s;

    HASH_FIND_STR(settings, key, s);  /* s: output pointer */
    return s;
}

void delete_setting(struct setting *st)
{
    HASH_DEL(settings, st);  /* st: pointer to deletee */
    free_setting(st);
}

void delete_all_setting() {
    struct setting *current_setting, *tmp;

    HASH_ITER(hh, settings, current_setting, tmp) {
        HASH_DEL(settings,current_setting);  /* delete it (users advances to next) */
        free_setting(current_setting);
    }
}

int main(int argc, char *argv[])
{
    if(argc > 1 && strcmp(argv[1], "--daemon") == 0){
        skeleton_daemon();
    }

    read_config_file(CONFIG_FILE_PATH);

    const int max_fd = sysconf(_SC_OPEN_MAX);

    struct pollfd           client[max_fd];
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
    int res = bind(listenfd, (struct sockaddr *)&servaddr, sizeof( servaddr ));
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

    for( i = 1 ; i < max_fd ; i ++ ){
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

            for( i = 1; i < max_fd; i ++ ){
                if( client[i].fd < 0 ){
                    client[i].fd = connfd;
                    break;
                }
            }

            if(i == max_fd){
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
                    struct setting * setting = find_setting(buf);
                    if(setting){
                        write(sockfd, setting->value, strlen(setting->value));
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
    delete_all_setting();
    syslog(LOG_NOTICE, PROJECT_NAME" terminated.");
    closelog();

    return EXIT_SUCCESS;
}


void read_config_file(const char *configFilename)
{
    FILE *configfp = fopen(configFilename, "r");
    if(!configfp){
        syslog(LOG_NOTICE, "File %s could not open errno: %d", configFilename, errno);
        return;
    }

    while (!feof(configfp)) {
        char buff[SBUF_SIZE] = {0};
        if (fgets(buff, sizeof(buff), configfp) != NULL){
            syslog(LOG_NOTICE, "Readed line from file is: %s", buff);
            char *pch = strchr(buff, '=');
            if(pch){
                int pos = pch - buff;
                buff[pos] = 0;
                char *key = buff;
                char *value = buff + pos + 1;
                add_setting(key, value);
            }
        }
    }

    fclose(configfp);
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
