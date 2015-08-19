#pragma once

#include "global/global.h"

#include "core/connection_types.h"

#include "common/net/net.h"

#define UNDEFINED_SINCE 0x00000000U
#define UNDEFINED_SINCE_STR "Undefined"
#define UNDEFINED_EXAMPLE_STR "Unspecified"
#define UNDEFINED_STR_IN_PROGRESS "Undefined in progress"
#define INFINITE_COMMAND_ARGS UINT8_MAX

namespace fastonosql
{
    struct CommandInfo
    {
        CommandInfo(const std::string& name, const std::string& params,
                    const std::string& summary, const uint32_t since,
                    const std::string& example, uint8_t required_arguments_count, uint8_t optional_arguments_count);

        uint16_t maxArgumentsCount() const;
        uint8_t minArgumentsCount() const;

        const std::string name_;
        const std::string params_;
        const std::string summary_;
        const uint32_t since_;
        const std::string example_;

        const uint8_t required_arguments_count_;
        const uint8_t optional_arguments_count_;
    };

    std::string convertVersionNumberToReadableString(uint32_t version);

    template<connectionTypes ct>
    struct DBTraits
    {
        static const std::vector<common::Value::Type> supportedTypes;
    };

    struct NKey
    {
        explicit NKey(const std::string& key, int32_t ttl_msec = -1);

        std::string key_;
        int32_t ttl_msec_;
    };

    inline bool operator == (const NKey& lhs, const NKey& rhs)
    {
        return lhs.key_ == rhs.key_;
    }

    struct NValue
    {
        NValue(common::Value::Type type = common::Value::TYPE_NULL);
        NValue(FastoObjectIPtr value);

        std::string toString() const;
        bool isValid() const;
        common::Value::Type type() const;

    private:
        FastoObjectIPtr value_;
        common::Value::Type type_;
    };

    class NDbValue
    {
    public:
        NDbValue(const NKey& key, const NValue& value);

        NKey key() const;
        NValue value() const;
        common::Value::Type type() const;

        void setTTL(int32_t ttl);
        void setValue(const NValue& value);

        std::string keyString() const;
        std::string valueString() const;

    private:
        NKey key_;
        NValue value_;
    };

    class ServerDiscoveryInfo
    {
    public:
        virtual ~ServerDiscoveryInfo();

        connectionTypes connectionType() const;
        serverTypes type() const;
        bool self() const;

        std::string name() const;
        void setName(const std::string& name);

        common::net::hostAndPort host() const;
        void setHost(const common::net::hostAndPort& host);

    protected:
        DISALLOW_COPY_AND_ASSIGN(ServerDiscoveryInfo);

        ServerDiscoveryInfo(connectionTypes ctype, serverTypes type, bool self);
        common::net::hostAndPort host_;
        std::string name_;

    private:
        const bool self_;
        const serverTypes type_;
        const connectionTypes ctype_;
    };

    typedef common::shared_ptr<ServerDiscoveryInfo> ServerDiscoveryInfoSPtr;

    class ServerInfo
    {
    public:
        ServerInfo(connectionTypes type);
        virtual ~ServerInfo();

        connectionTypes type() const;
        virtual std::string toString() const = 0;
        virtual uint32_t version() const = 0;
        virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const = 0;        

    protected:
        DISALLOW_COPY_AND_ASSIGN(ServerInfo);

    private:
        const connectionTypes type_;
    };

    struct FieldByIndex
    {
        virtual common::Value* valueByIndex(unsigned char index) const = 0;
    };

    struct Field
    {
        Field(const std::string& name, common::Value::Type type);

        bool isIntegral() const;
        std::string name_;
        common::Value::Type type_;
    };

    typedef common::shared_ptr<ServerInfo> ServerInfoSPtr;

    struct ServerInfoSnapShoot
    {
        ServerInfoSnapShoot();
        ServerInfoSnapShoot(common::time64_t msec, ServerInfoSPtr info);
        bool isValid() const;

        common::time64_t msec_;
        ServerInfoSPtr info_;
    };

    typedef std::pair<std::string, std::string> PropertyType;

    struct ServerPropertyInfo
    {
        ServerPropertyInfo();
        std::vector<PropertyType> propertyes_;
    };

    ServerPropertyInfo makeServerProperty(FastoObjectArray* array);

    class DataBaseInfo
    {
    public:
        typedef std::vector<NDbValue> keys_cont_type;
        connectionTypes type() const;
        std::string name() const;
        void setSize(size_t sz);
        size_t size() const;

        bool isDefault() const;
        void setIsDefault(bool isDef);

        virtual DataBaseInfo* clone() const = 0;
        virtual ~DataBaseInfo();

        keys_cont_type keys() const;
        void setKeys(const keys_cont_type& keys);

    protected:
        DataBaseInfo(const std::string& name, size_t size, bool isDefault, connectionTypes type);

    private:
        std::string name_;
        size_t size_;
        bool isDefault_;
        keys_cont_type keys_;

        const connectionTypes type_;
    };

    inline bool operator == (const DataBaseInfo& lhs, const DataBaseInfo& rhs)
    {
        return lhs.name() == rhs.name() && lhs.size() == rhs.size() && lhs.isDefault() == rhs.isDefault() && lhs.type() == rhs.type();
    }

    class CommandKey
    {
    public:
        enum cmdtype
        {
            C_NONE,
            C_DELETE,
            C_LOAD,
            C_CREATE
        };

        cmdtype type() const;
        NDbValue key() const;

        virtual ~CommandKey();

    protected:
        CommandKey(const NDbValue& key, cmdtype type);

        cmdtype type_;
        NDbValue key_;
    };

    class CommandDeleteKey
            : public CommandKey
    {
    public:
        CommandDeleteKey(const NDbValue& key);
    };

    class CommandLoadKey
            : public CommandKey
    {
    public:
        CommandLoadKey(const NDbValue& key);
    };

    class CommandCreateKey
            : public CommandKey
    {
    public:
        CommandCreateKey(const NDbValue& dbv);
        NValue value() const;
    };

    typedef common::shared_ptr<CommandKey> CommandKeySPtr;
}
