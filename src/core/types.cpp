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

    NKey::NKey(const std::string& key, int32_t ttl_sec)
        : key_(key), ttl_sec_(ttl_sec)
    {
    }

    NDbValue::NDbValue(const NKey& key, NValue value)
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
        if(!value_){
            return common::Value::TYPE_NULL;
        }

        return value_->getType();
    }

    void NDbValue::setTTL(int32_t ttl)
    {
        key_.ttl_sec_ = ttl;
    }

    void NDbValue::setValue(NValue value)
    {
        value_ = value;
    }

    std::string NDbValue::keyString() const
    {
        return key_.key_;
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

    std::vector<common::Value::Type> supportedTypesFromType(connectionTypes type)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            return DBTraits<REDIS>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type == MEMCACHED){
            return DBTraits<MEMCACHED>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type == SSDB){
            return DBTraits<SSDB>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type == LEVELDB){
            return DBTraits<LEVELDB>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_ROCKSDB
        if(type == ROCKSDB){
            return DBTraits<ROCKSDB>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_UNQLITE
        if(type == UNQLITE){
            return DBTraits<UNQLITE>::supportedTypes();
        }
#endif
#ifdef BUILD_WITH_LMDB
        if(type == LMDB){
            return DBTraits<LMDB>::supportedTypes();
        }
#endif
        NOTREACHED();
        return std::vector<common::Value::Type>();
    }

    std::vector<std::string> infoHeadersFromType(connectionTypes type)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            return DBTraits<REDIS>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type == MEMCACHED){
            return DBTraits<MEMCACHED>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type == SSDB){
            return DBTraits<SSDB>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type == LEVELDB){
            return DBTraits<LEVELDB>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_ROCKSDB
        if(type == ROCKSDB){
            return DBTraits<ROCKSDB>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_UNQLITE
        if(type == UNQLITE){
            return DBTraits<UNQLITE>::infoHeaders();
        }
#endif
#ifdef BUILD_WITH_LMDB
        if(type == LMDB){
            return DBTraits<LMDB>::infoHeaders();
        }
#endif
        NOTREACHED();
        return std::vector<std::string>();
    }

    std::vector< std::vector<Field> > infoFieldsFromType(connectionTypes type)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            return DBTraits<REDIS>::infoFields();
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type == MEMCACHED){
            return DBTraits<MEMCACHED>::infoFields();
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type == SSDB){
            return DBTraits<SSDB>::infoFields();
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type == LEVELDB){
            return DBTraits<LEVELDB>::infoFields();
        }
#endif
#ifdef BUILD_WITH_ROCKSDB
        if(type == ROCKSDB){
            return DBTraits<ROCKSDB>::infoFields();
        }
#endif
#ifdef BUILD_WITH_UNQLITE
        if(type == UNQLITE){
            return DBTraits<UNQLITE>::infoFields();
        }
#endif
#ifdef BUILD_WITH_LMDB
        if(type == LMDB){
            return DBTraits<LMDB>::infoFields();
        }
#endif
       NOTREACHED();
       return std::vector< std::vector<Field> >();
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

    CommandChangeTTL::CommandChangeTTL(const NDbValue& dbv, int32_t newTTL)
        : CommandKey(dbv, C_CHANGE_TTL), new_ttl_(newTTL)
    {

    }

    int32_t CommandChangeTTL::newTTL() const
    {
        return new_ttl_;
    }

    NDbValue CommandChangeTTL::newKey() const
    {
        NDbValue nk = key();
        nk.setTTL(new_ttl_);
        return nk;
    }

    NValue CommandCreateKey::value() const
    {
        return key_.value();
    }
}
