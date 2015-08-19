#include "core/types.h"

namespace fastonosql
{
    CommandInfo::CommandInfo(const std::string& name, const std::string& params,
                const std::string& summary, const uint32_t since, const std::string& example,
                             uint8_t required_arguments_count, uint8_t optional_arguments_count)
        : name_(name), params_(params), summary_(summary), since_(since), example_(example),
          required_arguments_count_(required_arguments_count), optional_arguments_count_(optional_arguments_count)
    {

    }

    uint16_t CommandInfo::maxArgumentsCount() const
    {
        return required_arguments_count_ + optional_arguments_count_;
    }

    uint8_t CommandInfo::minArgumentsCount() const
    {
        return required_arguments_count_;
    }

    std::string convertVersionNumberToReadableString(uint32_t version)
    {
        if(version != UNDEFINED_SINCE){
            return common::convertVersionNumberToString(version);
        }

        return UNDEFINED_SINCE_STR;
    }

    NKey::NKey(const std::string& key, int32_t ttl_msec)
        : key_(key), ttl_msec_(ttl_msec)
    {
    }

    NValue::NValue(common::Value::Type type)
        : type_(type), value_()
    {

    }

    NValue::NValue(FastoObjectIPtr value)
        : type_(common::Value::TYPE_NULL), value_(value)
    {

    }

    std::string NValue::toString() const
    {
        if(isValid()){
            return value_->toString();
        }

        NOTREACHED();
        return std::string();
    }

    bool NValue::isValid() const
    {
        return value_.get();
    }

    common::Value::Type NValue::type() const
    {
        if(!value_.get()){
            return type_;
        }

        return value_->type();
    }

    NDbValue::NDbValue(const NKey& key, const NValue& value)
        : key_(key), value_(value)
    {

    }

    NKey NDbValue::key() const
    {
        return key_;
    }

    NValue NDbValue::value() const
    {
        return value_;
    }

    common::Value::Type NDbValue::type() const
    {
        return value_.type();
    }

    void NDbValue::setTTL(int32_t ttl)
    {
        key_.ttl_msec_ = ttl;
    }

    void NDbValue::setValue(const NValue& value)
    {
        value_ = value;
    }

    std::string NDbValue::keyString() const
    {
        return key_.key_;
    }

    std::string NDbValue::valueString() const
    {
        return value_.toString();
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

    CommandKey::CommandKey(const NDbValue &key, cmdtype type)
        : type_(type), key_(key)
    {

    }

    CommandKey::cmdtype CommandKey::type() const
    {
        return type_;
    }

    NDbValue CommandKey::key() const
    {
        return key_;
    }

    CommandKey::~CommandKey()
    {

    }

    CommandDeleteKey::CommandDeleteKey(const NDbValue &key)
        : CommandKey(key, C_DELETE)
    {

    }

    CommandLoadKey::CommandLoadKey(const NDbValue &key)
        : CommandKey(key, C_LOAD)
    {

    }

    CommandCreateKey::CommandCreateKey(const NDbValue& dbv)
        : CommandKey(dbv, C_CREATE)
    {

    }

    NValue CommandCreateKey::value() const
    {
        return key_.value();
    }
}
