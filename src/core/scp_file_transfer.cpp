#include "core/scp_file_transfer.h"

#ifdef OS_WIN
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#ifdef OS_FREEBSD
#include <netinet/in.h>
#endif
#endif

#include "third-party/libssh2/include/libssh2.h"

#include "fasto/qt/logger.h"

namespace fastoredis
{
    int scp_send_file(const std::string& username, const std::string& password, const std::string& loclfile, const std::string& host, const std::string& scppath)
    {
        if(username.empty()){
            return ERROR_RESULT_VALUE;
        }

        if(password.empty()){
            return ERROR_RESULT_VALUE;
        }

        if(loclfile.empty()){
            return ERROR_RESULT_VALUE;
        }

        if(scppath.empty()){
            return ERROR_RESULT_VALUE;
        }

        if(host.empty()){
            return ERROR_RESULT_VALUE;
        }

        FILE *local = NULL;
        LIBSSH2_CHANNEL* channel = NULL;
        char mem[1024] = {0};

        int rc = libssh2_init(0);

        if (rc != 0) {
            char buff[256] = {0};
            sprintf(buff, "libssh2 initialization failed (%d)", rc);
            common::ErrorValueSPtr er = common::make_error_value(buff, common::Value::E_ERROR);
            LOG_ERROR(er, true);
            return ERROR_RESULT_VALUE;
        }

        local = fopen(loclfile.c_str(), "rb");
        if (!local) {
            char buff[256] = {0};
            sprintf(buff, "Can't open local file %s", loclfile.c_str());
            common::ErrorValueSPtr er = common::make_error_value(buff, common::Value::E_ERROR);
            LOG_ERROR(er, true);
            return ERROR_RESULT_VALUE;
        }

        struct stat fileinfo;
        stat(loclfile.c_str(), &fileinfo);

        /* Ultra basic "connect to port 22 on localhost"
         * Your code is responsible for creating the socket establishing the
         * connection
         */
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == INVALID_DESCRIPTOR) {
            common::ErrorValueSPtr er = common::make_error_value("failed to create socket!", common::Value::E_ERROR);
            LOG_ERROR(er, true);
            return ERROR_RESULT_VALUE;
        }

        unsigned long hostaddr = inet_addr(host.c_str());

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(22);
        sin.sin_addr.s_addr = hostaddr;
        if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
            common::ErrorValueSPtr er = common::make_error_value("failed to connect!", common::Value::E_ERROR);
            LOG_ERROR(er, true);
            return ERROR_RESULT_VALUE;
        }

        /* Create a session instance
         */
        LIBSSH2_SESSION *session = libssh2_session_init();

        if(!session){
            return ERROR_RESULT_VALUE;
        }

        /* ... start it up. This will trade welcome banners, exchange keys,
        * and setup crypto, compression, and MAC layers
        */
       rc = libssh2_session_handshake(session, sock);

       if(rc) {
           char buff[256] = {0};
           sprintf(buff, "Failure establishing SSH session: %d\n", rc);
           common::ErrorValueSPtr er = common::make_error_value(buff, common::Value::E_ERROR);
           LOG_ERROR(er, true);
           return ERROR_RESULT_VALUE;
       }

       libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

       /* We could authenticate via password */
       if (libssh2_userauth_password(session, username.c_str(), password.c_str())) {
           common::ErrorValueSPtr er = common::make_error_value("Authentication by password failed.", common::Value::E_ERROR);
           LOG_ERROR(er, true);
           goto shutdown;
       }

       /* Send a file via scp. The mode parameter must only have permissions! */
       channel = libssh2_scp_send(session, scppath.c_str(), fileinfo.st_mode & 0777, (unsigned long)fileinfo.st_size);
       if (!channel) {
           char *errmsg;
           int errlen;
           int err = libssh2_session_last_error(session, &errmsg, &errlen, 0);

           char buffc[256] = {0};
           sprintf(buffc, "Unable to open a session: (%d) %s\n", err, errmsg);
           common::ErrorValueSPtr er = common::make_error_value(buffc, common::Value::E_ERROR);
           LOG_ERROR(er, true);
           goto shutdown;
       }

       do {
           int nread = fread(mem, 1, sizeof(mem), local);
           if (nread <= 0) {
               /* end of file */
               break;
           }
           char *ptr = mem;

           do {
               /* write the same data over and over, until error or completion */
               rc = libssh2_channel_write(channel, ptr, nread);

               if (rc < 0) {
                   char buff[256] = {0};
                   sprintf(buff, "ERROR %d\n", rc);
                   common::ErrorValueSPtr er = common::make_error_value(buff, common::Value::E_ERROR);
                   LOG_ERROR(er, true);
                   break;
               }
               else {
                   /* rc indicates how many bytes were written this time */
                   ptr += rc;
                   nread -= rc;
               }
           } while (nread);

       } while (1);

       libssh2_channel_send_eof(channel);
       libssh2_channel_wait_eof(channel);
       libssh2_channel_wait_closed(channel);
       libssh2_channel_free(channel);
       channel = NULL;

shutdown:

       if(session) {
           libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
           libssh2_session_free(session);
       }

    #ifdef WIN32
       closesocket(sock);
    #else
       close(sock);
    #endif
       if (local){
           fclose(local);
       }
       libssh2_exit();


       return 0;
    }
}
