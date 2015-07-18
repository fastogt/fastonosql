#pragma once

#include "core/core_fwd.h"
#include "common/qt/utils_qt.h"

namespace fastoredis
{
    namespace EventsInfo
    {
        struct EventInfoBase
                : public common::utils_qt::EventInfo<common::ErrorValueSPtr>
        {
            typedef common::utils_qt::EventInfo<common::ErrorValueSPtr > base_class;
            EventInfoBase(const error_type &er = error_type());
            EventInfoBase(const common::time64_t time_start, const error_type &er = error_type());
            common::time64_t elapsedTime() const;

        private:
            const common::time64_t time_start_;
        };

        struct ConnectInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ConnectInfoRequest(const error_type &er = error_type());
        };

        struct ConnectInfoResponce
                : ConnectInfoRequest
        {
            typedef ConnectInfoRequest base_class;
            ConnectInfoResponce(const base_class &request);
        };

        struct ShutDownInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ShutDownInfoRequest(const error_type &er = error_type());
        };

        struct ShutDownInfoResponce
                : ShutDownInfoRequest
        {
            typedef ShutDownInfoRequest base_class;
            ShutDownInfoResponce(const base_class &request);
        };

        struct BackupInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            BackupInfoRequest(const std::string& path, const error_type &er = error_type());
            std::string path_;
        };

        struct BackupInfoResponce
                : EventInfoBase
        {
            typedef EventInfoBase base_class;
            BackupInfoResponce(const base_class &request);
        };

        struct ExportInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ExportInfoRequest(const std::string& path, const error_type &er = error_type());
            std::string path_;
        };

        struct ExportInfoResponce
                : ExportInfoRequest
        {
            typedef ExportInfoRequest base_class;
            ExportInfoResponce(const base_class &request);
        };

        struct ChangePasswordRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ChangePasswordRequest(const std::string& oldPassword, const std::string& newPassword, const error_type &er = error_type());
            std::string oldPassword_;
            std::string newPassword_;
        };

        struct ChangePasswordResponce
                : ChangePasswordRequest
        {
            typedef ChangePasswordRequest base_class;
            ChangePasswordResponce(const base_class& request);
        };

        struct ChangeMaxConnectionRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ChangeMaxConnectionRequest(int maxConnection, const error_type &er = error_type());
            int maxConnection_;
        };

        struct ChangeMaxConnectionResponce
                : ChangeMaxConnectionRequest
        {
            typedef ChangeMaxConnectionRequest base_class;
            ChangeMaxConnectionResponce(const base_class& request);
        };

        struct ProcessConfigArgsInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ProcessConfigArgsInfoRequest(const error_type &er = error_type());
        };

        struct ProcessConfigArgsInfoResponce
                : ProcessConfigArgsInfoRequest
        {
            typedef ProcessConfigArgsInfoRequest base_class;
            ProcessConfigArgsInfoResponce(const base_class &request);
        };

        struct DiscoveryInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            DiscoveryInfoRequest(const error_type &er = error_type());
        };

        struct DiscoveryInfoResponce
                : DiscoveryInfoRequest
        {
            typedef DiscoveryInfoRequest base_class;
            DiscoveryInfoResponce(const base_class &request);

            ServerDiscoveryInfoSPtr info_;
        };

        struct EnterModeInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            EnterModeInfo(ConnectionMode mode, const error_type &er = error_type());
            ConnectionMode mode_;
        };

        struct LeaveModeInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            LeaveModeInfo(ConnectionMode mode, const error_type &er = error_type());
            ConnectionMode mode_;
        };

        struct CommandRootCreatedInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRootCreatedInfo(FastoObjectIPtr root, const error_type &er = error_type());

            FastoObjectIPtr root_;
        };

        struct CommandRootCompleatedInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRootCompleatedInfo(FastoObjectIPtr root, const error_type &er = error_type());
            CommandRootCompleatedInfo(common::time64_t timest, FastoObjectIPtr root, const error_type &er = error_type());

            FastoObjectIPtr root_;
        };

        struct DisonnectInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            DisonnectInfoRequest(const error_type &er = error_type());
        };

        struct DisConnectInfoResponce
                : DisonnectInfoRequest
        {
            typedef DisonnectInfoRequest base_class;
            DisConnectInfoResponce(const base_class &request);
        };

        struct ExecuteInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ExecuteInfoRequest(const std::string &text, const std::vector<std::string>& args = std::vector<std::string>(), const error_type &er = error_type());

            const std::string text_;
            const std::vector<std::string> args_;
        };

        struct LoadDatabasesInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            LoadDatabasesInfoRequest(const error_type &er = error_type());
        };

        struct LoadDatabasesInfoResponce
                : LoadDatabasesInfoRequest
        {
            typedef LoadDatabasesInfoRequest base_class;
            typedef std::vector<DataBaseInfoSPtr> database_info_cont_type;
            LoadDatabasesInfoResponce(const base_class &request);

            database_info_cont_type databases_;
        };

        struct LoadDatabaseContentRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            LoadDatabaseContentRequest(DataBaseInfoSPtr inf, const std::string& pattern, uint32_t countKeys,
                                       uint32_t cursor, const error_type &er = error_type());

            DataBaseInfoSPtr inf_;
            std::string pattern_;
            uint32_t countKeys_;
            const uint32_t cursorIn_;
        };

        struct LoadDatabaseContentResponce
                : LoadDatabaseContentRequest
        {
            typedef LoadDatabaseContentRequest base_class;
            typedef std::vector<NKey> keys_cont_type;
            LoadDatabaseContentResponce(const base_class &request);

            keys_cont_type keys_;
            uint32_t cursorOut_;
        };

        struct SetDefaultDatabaseRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            SetDefaultDatabaseRequest(DataBaseInfoSPtr inf, const error_type &er = error_type());

            DataBaseInfoSPtr inf_;
        };

        struct SetDefaultDatabaseResponce
                : SetDefaultDatabaseRequest
        {
            typedef SetDefaultDatabaseRequest base_class;
            SetDefaultDatabaseResponce(const base_class &request);
        };

        struct ServerInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ServerInfoRequest(const error_type &er = error_type());
        };

        struct ServerInfoResponce
                : ServerInfoRequest
        {
            typedef ServerInfoRequest base_class;
            ServerInfoResponce(const base_class &request);
            ~ServerInfoResponce();

            ServerInfoSPtr info() const;
            void setInfo(ServerInfoSPtr inf);

        private:
            ServerInfoSPtr info_;
        };

        struct ServerInfoHistoryRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ServerInfoHistoryRequest(const error_type &er = error_type());
        };

        struct ServerInfoHistoryResponce
                : ServerInfoHistoryRequest
        {
            typedef ServerInfoHistoryRequest base_class;
            typedef std::vector<ServerInfoSnapShoot> infos_container_type;
            ServerInfoHistoryResponce(const base_class &request);

            infos_container_type infos() const;
            void setInfos(const infos_container_type& inf);

        private:
            infos_container_type infos_;
        };

        struct ServerPropertyInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ServerPropertyInfoRequest(const error_type &er = error_type());
        };

        struct ServerPropertyInfoResponce
                : ServerPropertyInfoRequest
        {
            typedef ServerPropertyInfoRequest base_class;
            ServerPropertyInfoResponce(const base_class &request);

            ServerPropertyInfo info_;
        };

        struct ChangeServerPropertyInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ChangeServerPropertyInfoRequest(const error_type &er = error_type());

            PropertyType newItem_;
        };

        struct ChangeServerPropertyInfoResponce
                : ChangeServerPropertyInfoRequest
        {
            typedef ChangeServerPropertyInfoRequest base_class;
            ChangeServerPropertyInfoResponce(const base_class &request);

            bool isChange_;
        };

        struct ChangeDbValueRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ChangeDbValueRequest(const NDbValue& val, const error_type &er = error_type());

            NDbValue newItem_;
            std::string command_;
        };

        struct ChangeDbValueResponce
                : ChangeDbValueRequest
        {
            typedef ChangeDbValueRequest base_class;
            ChangeDbValueResponce(const base_class &request);

            bool isChange_;
        };

        struct CommandRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRequest(DataBaseInfoSPtr inf, CommandKeySPtr cmd, const error_type &er = error_type());

            DataBaseInfoSPtr inf_;
            CommandKeySPtr cmd_;
        };

        struct CommandResponce
                : CommandRequest
        {
            typedef CommandRequest base_class;
            CommandResponce(const base_class &request);
        };

        struct ProgressInfoResponce
        {
            ProgressInfoResponce(uint8_t pr);

            const uint8_t progress_;
        };
    }
}
