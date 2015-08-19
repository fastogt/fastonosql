#pragma once

#include "core/core_fwd.h"
#include "common/qt/utils_qt.h"

namespace fastonosql
{
    namespace EventsInfo
    {
        struct EventInfoBase
                : public common::utils_qt::EventInfo<common::ErrorValueSPtr>
        {
            typedef common::utils_qt::EventInfo<common::ErrorValueSPtr > base_class;
            explicit EventInfoBase(initiator_type sender, const error_type &er = error_type());
            EventInfoBase(initiator_type sender, const common::time64_t time_start, const error_type &er = error_type());
            common::time64_t elapsedTime() const;

        private:
            const common::time64_t time_start_;
        };

        struct ConnectInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            explicit ConnectInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            explicit ShutDownInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            BackupInfoRequest(initiator_type sender, const std::string& path, const error_type &er = error_type());
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
            ExportInfoRequest(initiator_type sender, const std::string& path, const error_type &er = error_type());
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
            ChangePasswordRequest(initiator_type sender, const std::string& oldPassword, const std::string& newPassword, const error_type &er = error_type());
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
            ChangeMaxConnectionRequest(initiator_type sender, int maxConnection, const error_type &er = error_type());
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
            explicit ProcessConfigArgsInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            explicit DiscoveryInfoRequest(initiator_type sender, const error_type &er = error_type());
        };

        struct DiscoveryInfoResponce
                : DiscoveryInfoRequest
        {
            typedef DiscoveryInfoRequest base_class;
            DiscoveryInfoResponce(const base_class &request);

            ServerInfoSPtr sinfo_;
            ServerDiscoveryInfoSPtr dinfo_;
            DataBaseInfoSPtr dbinfo_;
        };

        struct EnterModeInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            EnterModeInfo(initiator_type sender, ConnectionMode mode, const error_type &er = error_type());
            ConnectionMode mode_;
        };

        struct LeaveModeInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            LeaveModeInfo(initiator_type sender, ConnectionMode mode, const error_type &er = error_type());
            ConnectionMode mode_;
        };

        struct CommandRootCreatedInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRootCreatedInfo(initiator_type sender, FastoObjectIPtr root, const error_type &er = error_type());

            FastoObjectIPtr root_;
        };

        struct CommandRootCompleatedInfo
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRootCompleatedInfo(initiator_type sender, FastoObjectIPtr root, const error_type &er = error_type());
            CommandRootCompleatedInfo(initiator_type sender, common::time64_t timest, FastoObjectIPtr root, const error_type &er = error_type());

            FastoObjectIPtr root_;
        };

        struct DisConnectInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            DisConnectInfoRequest(initiator_type sender, const error_type &er = error_type());
        };

        struct DisConnectInfoResponce
                : DisConnectInfoRequest
        {
            typedef DisConnectInfoRequest base_class;
            DisConnectInfoResponce(const base_class &request);
        };

        struct ExecuteInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            ExecuteInfoRequest(initiator_type sender, const std::string &text, const std::vector<std::string>& args = std::vector<std::string>(), const error_type &er = error_type());

            const std::string text_;
            const std::vector<std::string> args_;
        };

        struct LoadDatabasesInfoRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            explicit LoadDatabasesInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            LoadDatabaseContentRequest(initiator_type sender, DataBaseInfoSPtr inf, const std::string& pattern, uint32_t countKeys,
                                       uint32_t cursor = 0, const error_type &er = error_type());

            DataBaseInfoSPtr inf_;
            std::string pattern_;
            uint32_t countKeys_;
            const uint32_t cursorIn_;
        };

        struct LoadDatabaseContentResponce
                : LoadDatabaseContentRequest
        {
            typedef LoadDatabaseContentRequest base_class;
            typedef std::vector<NDbValue> keys_cont_type;
            LoadDatabaseContentResponce(const base_class &request);

            keys_cont_type keys_;
            uint32_t cursorOut_;
        };

        struct SetDefaultDatabaseRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            SetDefaultDatabaseRequest(initiator_type sender, DataBaseInfoSPtr inf, const error_type &er = error_type());

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
            explicit ServerInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            explicit ServerInfoHistoryRequest(initiator_type sender, const error_type &er = error_type());
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
            explicit ServerPropertyInfoRequest(initiator_type sender, const error_type &er = error_type());
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
            ChangeServerPropertyInfoRequest(initiator_type sender, const PropertyType& pt, const error_type &er = error_type());

            PropertyType newItem_;
        };

        struct ChangeServerPropertyInfoResponce
                : ChangeServerPropertyInfoRequest
        {
            typedef ChangeServerPropertyInfoRequest base_class;
            ChangeServerPropertyInfoResponce(const base_class &request);

            bool isChange_;
        };

        struct CommandRequest
                : public EventInfoBase
        {
            typedef EventInfoBase base_class;
            CommandRequest(initiator_type sender, DataBaseInfoSPtr inf, CommandKeySPtr cmd, const error_type &er = error_type());

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
