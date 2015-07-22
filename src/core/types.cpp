#include "core/types.h"

namespace fastonosql
{
    NKey::NKey(const std::string& key, common::Value::Type type)
        : key_(key), type_(type)
    {
    }

    NValue::NValue(const std::string& value, common::Value::Type type)
        : value_(value), type_(type)
    {

    }

    NDbValue::NDbValue(const std::string& key, const std::string& value, common::Value::Type type)
        : key_(key), value_(value), type_(type)
    {
    }

    NKey NDbValue::key() const
    {
        return NKey(key_, type_);
    }

    NValue NDbValue::value() const
    {
        return NValue(value_, type_);
    }

    std::string NDbValue::keyString() const
    {
        return key_;
    }

    std::string NDbValue::valueString() const
    {
        return value_;
    }

    ServerDiscoveryInfo::ServerDiscoveryInfo(connectionTypes ctype, serverTypes type, bool self)
        : host_(), name_(), self_(self), type_(type), ctype_(ctype)
    {

    }

    connectionTypes ServerDiscoveryInfo::connectionType() const
    {
        return ctype_;
    }

    serverTypes ServerDiscoveryInfo::type() const
    {
        return type_;
    }

    bool ServerDiscoveryInfo::self() const
    {
        return self_;
    }

    std::string ServerDiscoveryInfo::name() const
    {
        return name_;
    }

    void ServerDiscoveryInfo::setName(const std::string& name)
    {
        name_ = name;
    }

    common::net::hostAndPort ServerDiscoveryInfo::host() const
    {
        return host_;
    }

    void ServerDiscoveryInfo::setHost(const common::net::hostAndPort& host)
    {
        host_ = host;
    }


    ServerDiscoveryInfo::~ServerDiscoveryInfo()
    {

    }

    ServerInfo::~ServerInfo()
    {

    }

    connectionTypes ServerInfo::type() const
    {
        return type_;
    }

    ServerInfo::ServerInfo(connectionTypes type)
        : type_(type)
    {

    }

    Field::Field(const std::string& name, common::Value::Type type)
        : name_(name), type_(type)
    {

    }

    bool Field::isIntegral() const
    {
        return common::Value::isIntegral(type_);
    }

    ServerInfoSnapShoot::ServerInfoSnapShoot()
        : msec_(0), info_()
    {

    }

    ServerInfoSnapShoot::ServerInfoSnapShoot(common::time64_t msec, ServerInfoSPtr info)
        : msec_(msec), info_(info)
    {

    }

    bool ServerInfoSnapShoot::isValid() const
    {
        return msec_ > 0 && info_;
    }

    ServerPropertyInfo::ServerPropertyInfo()
    {

    }

    ServerPropertyInfo makeServerProperty(FastoObjectArray* array)
    {
        ServerPropertyInfo inf;

        common::ArrayValue* ar = array->array();
        if(ar){
            for(int i = 0; i < ar->getSize(); i+=2){
                std::string c1;
                std::string c2;
                bool res = ar->getString(i, &c1);
                DCHECK(res);
                res = ar->getString(i+1, &c2);
                DCHECK(res);
                inf.propertyes_.push_back(std::make_pair(c1, c2));
            }
        }
        return inf;
    }

    DataBaseInfo::DataBaseInfo(const std::string& name, size_t size, bool isDefault, connectionTypes type)
        : name_(name), size_(size), isDefault_(isDefault), type_(type)
    {

    }

    DataBaseInfo::~DataBaseInfo()
    {

    }

    connectionTypes DataBaseInfo::type() const
    {
        return type_;
    }

    std::string DataBaseInfo::name() const
    {
        return name_;
    }

    void DataBaseInfo::setSize(size_t sz)
    {
        size_ = sz;
    }

    size_t DataBaseInfo::size() const
    {
        return size_;
    }

    bool DataBaseInfo::isDefault() const
    {
        return isDefault_;
    }

    void DataBaseInfo::setIsDefault(bool isDef)
    {
        isDefault_ = isDef;
    }

    void DataBaseInfo::setKeys(const keys_cont_type& keys)
    {
        keys_ = keys;
    }

    DataBaseInfo::keys_cont_type DataBaseInfo::keys() const
    {
        return keys_;
    }

    CommandKey::CommandKey(const NKey& key, cmdtype type)
        : type_(type), key_(key)
    {

    }

    CommandKey::cmdtype CommandKey::type() const
    {
        return type_;
    }

    NKey CommandKey::key() const
    {
        return key_;
    }

    CommandKey::~CommandKey()
    {

    }

    CommandDeleteKey::CommandDeleteKey(const NKey& key)
        : CommandKey(key, C_DELETE)
    {

    }

    CommandLoadKey::CommandLoadKey(const NKey& key)
        : CommandKey(key, C_LOAD)
    {

    }

    CommandCreateKey::CommandCreateKey(const NKey& key, FastoObjectIPtr value)
        : CommandKey(key, C_CREATE), value_(value)
    {

    }

    FastoObjectIPtr CommandCreateKey::value() const
    {
        return value_;
    }
}
