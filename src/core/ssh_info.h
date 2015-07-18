#pragma once

#include <string>

#define DEFAULT_SSH_PORT 22
#define DEFAULT_SSH_HOST ""

namespace fastoredis
{
    struct SSHInfo
    {
        enum SupportedAuthenticationMetods
        {
            UNKNOWN = 0,
            PASSWORD = 1,
            PUBLICKEY = 2
        };

        SSHInfo();
        SSHInfo(const std::string& hostName, int port, const std::string& userName, const std::string& password,
                const std::string& publicKey, const std::string& privateKey, const std::string& passphrase,
                SupportedAuthenticationMetods method);

        SSHInfo(const std::string& text);

        bool isValid() const;
        SupportedAuthenticationMetods authMethod() const;

        std::string toString() const;

        std::string hostName_;
        int port_;
        std::string userName_;
        std::string password_;
        std::string publicKey_;
        std::string privateKey_;
        std::string passphrase_;

        SupportedAuthenticationMetods currentMethod_;
    };

    inline bool operator == (const SSHInfo& r,const SSHInfo& l)
    {
        return r.hostName_ == l.hostName_ && r.password_ == l.password_ && r.port_ == l.port_ && r.publicKey_ == l.publicKey_ && r.privateKey_ == l.privateKey_ && r.passphrase_ == l.passphrase_ && r.userName_ == l.userName_;
    }
}
