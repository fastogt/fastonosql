#include "global/global.h"

#include "common/string_util.h"

namespace fastoredis
{
    FastoObject::FastoObject(FastoObject* parent, common::Value* val, const std::string& delemitr)
        : observer_(NULL), value_(val), parent_(parent), childrens_(), delemitr_(delemitr)
    {
        DCHECK(val);
    }

    FastoObject::~FastoObject()
    {
        for(int i = 0; i < childrens_.size(); ++i){
            FastoObject* item = childrens_[i];
            delete item;
        }
        childrens_.clear();
    }

    common::Value::Type FastoObject::type() const
    {
        return value_->getType();
    }

    std::string FastoObject::toString() const
    {
        std::string result = value_->toString();//getAsString(&result);
        return result;
    }

    FastoObject* FastoObject::createRoot(const std::string &text, IFastoObjectObserver* observer)
    {
        FastoObject* root =  new FastoObject(NULL, common::Value::createStringValue(text), "");
        root->observer_ = observer;
        return root;
    }

    FastoObject::child_container_type FastoObject::childrens() const
    {
        return childrens_;
    }

    void FastoObject::addChildren(FastoObject* child)
    {
        if(child){
            DCHECK(child->parent_ == this);
            childrens_.push_back(child);
            if(observer_){
                observer_->addedChildren(child);
                child->observer_ = observer_;
            }
        }
    }

    FastoObject* FastoObject::parent() const
    {
        return parent_;
    }

    void FastoObject::clear()
    {
        for (child_container_type::const_iterator it = childrens_.begin(); it != childrens_.end(); ++it){
            FastoObject* child = (*it);
            delete child;
        }
        childrens_.clear();
    }

    std::string FastoObject::delemitr() const
    {
        return delemitr_;
    }

    common::Value* FastoObject::value() const
    {
        return value_.get();
    }

    void FastoObject::setValue(common::Value* val)
    {
        value_.reset(val);
        if(observer_){
            observer_->updated(this, val);
        }
    }

    FastoObjectCommand::FastoObjectCommand(FastoObject* parent, common::CommandValue* cmd, const std::string& delemitr)
        : FastoObject(parent, cmd, delemitr)
    {

    }

    FastoObjectCommand::~FastoObjectCommand()
    {

    }

    common::CommandValue* FastoObjectCommand::cmd() const
    {
        return dynamic_cast<common::CommandValue*>(value_.get());
    }

    std::string FastoObjectCommand::toString() const
    {
        return std::string();
    }

    std::string FastoObjectCommand::inputCommand() const
    {
        common::CommandValue* command = cmd();
        if(command){
            return command->inputCommand();
        }

        return std::string();
    }

    std::string FastoObjectCommand::oppositeCommand() const
    {
        common::CommandValue* command = cmd();
        if(command){
            return command->oppositeCommand();
        }

        return std::string();
    }

    common::Value::CommandType FastoObjectCommand::commandType() const
    {
        common::CommandValue* command = cmd();
        if(command){
            return command->commandType();
        }

        return common::Value::C_UNKNOWN;
    }

    std::string stableCommand(const char* command)
    {
        if(!command){
            return std::string();
        }

        std::string cmd = command;

        if(cmd[cmd.size() - 1] == '\r'){
            cmd.resize(cmd.size() - 1);
        }

        return cmd;
    }

    std::pair<std::string, std::string> getKeyValueFromLine(const std::string& input)
    {
        if(input.empty()){
            return std::pair<std::string, std::string>();
        }

        size_t pos = input.find_first_of(' ');
        std::string key = input;
        std::string value;
        if(pos != std::string::npos){
            key = input.substr(0, pos);
            value = input.substr(pos + 1);
        }

        std::string trimed;
        common::TrimWhitespaceASCII(value, common::TRIM_ALL, &trimed);
        return std::make_pair(key, value);
    }

    std::string getOppositeCommand(const std::string& command, const std::vector<std::pair<std::string, std::string > >& srcOppositeCommands)
    {
        DCHECK(!command.empty());
        if(command.empty()){
            return std::string();
        }

        std::string uppercmd = StringToUpperASCII(command);
        for(int i = 0; i < srcOppositeCommands.size(); ++i){
            std::pair<std::string, std::string > p = srcOppositeCommands[i];
            if(p.first == uppercmd){
                return p.second;
            }
            else if(p.second == uppercmd){
                return p.first;
            }
        }

        return std::string();
    }

    FastoObjectArray::FastoObjectArray(FastoObject* parent, common::ArrayValue* ar, const std::string& delemitr)
        : FastoObject(parent, ar, delemitr)
    {

    }

    void FastoObjectArray::append(common::Value* in_value)
    {
        common::ArrayValue* ar = dynamic_cast<common::ArrayValue*>(value_.get());
        if(!ar){
            NOTREACHED();
            return;
        }

        ar->append(in_value);
    }

    std::string FastoObjectArray::toString() const
    {
        common::ArrayValue* ar = dynamic_cast<common::ArrayValue*>(value_.get());
        if(!ar){
            return std::string();
        }

        std::string result;
        const common::ArrayValue::const_iterator lastIt = std::prev(ar->end());
        for(common::ArrayValue::const_iterator it = ar->begin(); it != ar->end(); ++it){
            std::string val = (*it)->toString();
            if(val.empty()){
                continue;
            }

            result += val;
            if(lastIt != it){
                result += delemitr();
            }
        }
        return result;
    }

    common::ArrayValue* FastoObjectArray::array() const
    {
        return dynamic_cast<common::ArrayValue*>(value_.get());
    }

    FastoObjectSet::FastoObjectSet(FastoObject* parent, common::SetValue* set, const std::string& delemitr)
        : FastoObject(parent, set, delemitr)
    {

    }

    // Insert a Value to the set.
    void FastoObjectSet::insert(common::Value* in_value)
    {
        common::SetValue* ar = set();
        if(!ar){
            NOTREACHED();
            return;
        }

        ar->insert(in_value);
    }

    std::string FastoObjectSet::toString() const
    {
        common::SetValue* ar = set();
        if(!ar){
            return std::string();
        }

        std::string result;
        const common::SetValue::const_iterator lastIt = std::prev(ar->end());
        for(common::SetValue::const_iterator it = ar->begin(); it != ar->end(); ++it){
            std::string val = (*it)->toString();
            if(val.empty()){
                continue;
            }

            result += val;
            if(lastIt != it){
                result += delemitr();
            }
        }
        return result;
    }

    common::SetValue* FastoObjectSet::set() const
    {
        return dynamic_cast<common::SetValue*>(value_.get());
    }

    FastoObjectZSet::FastoObjectZSet(FastoObject* parent, common::ZSetValue* ar, const std::string& delemitr)
        : FastoObject(parent, ar, delemitr)
    {

    }

    void FastoObjectZSet::insert(common::Value* key, common::Value* value)
    {
        common::ZSetValue* ar = zset();
        if(!ar){
            NOTREACHED();
            return;
        }

        ar->insert(key, value);
    }

    std::string FastoObjectZSet::toString() const
    {
        common::ZSetValue* ar = zset();
        if(!ar){
            return std::string();
        }

        std::string result;
        const common::ZSetValue::const_iterator lastIt = std::prev(ar->end());
        for(common::ZSetValue::const_iterator it = ar->begin(); it != ar->end(); ++it){
            common::ZSetValue::value_type v = *it;
            std::string key = (v.first)->toString();
            std::string val = (v.second)->toString();
            if(val.empty() || key.empty()){
                continue;
            }

            result += key + " " + val;
            if(lastIt != it){
                result += delemitr();
            }
        }
        return result;
    }

    common::ZSetValue* FastoObjectZSet::zset() const
    {
        return dynamic_cast<common::ZSetValue*>(value_.get());
    }

    FastoObjectHash::FastoObjectHash(FastoObject* parent, common::HashValue *ar, const std::string& delemitr)
        : FastoObject(parent, ar, delemitr)
    {

    }

    // Insert a Value to the map.
    void FastoObjectHash::insert(common::Value* key, common::Value* value)
    {
        common::HashValue* ar = hash();
        if(!ar){
            NOTREACHED();
            return;
        }

        ar->insert(key, value);
    }

    std::string FastoObjectHash::toString() const
    {
        common::HashValue* ar = hash();
        if(!ar){
            return std::string();
        }

        std::string result;
        for(common::HashValue::const_iterator it = ar->begin(); it != ar->end(); ++it){
            common::HashValue::value_type v = *it;
            std::string key = (v.first)->toString();
            std::string val = (v.second)->toString();
            if(val.empty() || key.empty()){
                continue;
            }

            result += key + " " + val;
            if(std::next(it) != ar->end()){
                result += delemitr();
            }
        }
        return result;
    }

    common::HashValue* FastoObjectHash::hash() const
    {
        return dynamic_cast<common::HashValue*>(value_.get());
    }
}

namespace common
{
    std::string convertToString(fastoredis::FastoObject* obj)
    {
        using namespace fastoredis;
        std::string result;
        if(obj){
            const std::string str = obj->toString();
            if(!str.empty()){
                result += str + obj->delemitr();
            }
            FastoObject::child_container_type childrens = obj->childrens();
            for(FastoObject::child_container_type::const_iterator it = childrens.begin(); it != childrens.end(); ++it ){
                result += convertToString(*it);
            }
        }
        return result;
    }
}
