#include "core/events/events_info.h"

#include "common/time.h"

namespace fastonosql
{
    namespace EventsInfo
    {
        EventInfoBase::EventInfoBase(initiator_type sender, const error_type &er)
            : base_class(sender, er), time_start_(common::time::current_mstime())
        {

        }

        EventInfoBase::EventInfoBase(initiator_type sender, const common::time64_t time_start, const error_type &er)
            : base_class(sender, er), time_start_(time_start)
        {

        }

        common::time64_t EventInfoBase::elapsedTime() const
        {
            return common::time::current_mstime() - time_start_;
        }

        ConnectInfoRequest::ConnectInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        ConnectInfoResponce::ConnectInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        ShutDownInfoRequest::ShutDownInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        ShutDownInfoResponce::ShutDownInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        BackupInfoRequest::BackupInfoRequest(initiator_type sender, const std::string& path, const error_type &er)
            : base_class(sender, er), path_(path)
        {

        }

        BackupInfoResponce::BackupInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        ExportInfoRequest::ExportInfoRequest(initiator_type sender, const std::string& path, const error_type& er)
            : base_class(sender, er), path_(path)
        {

        }

        ExportInfoResponce::ExportInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        ChangePasswordRequest::ChangePasswordRequest(initiator_type sender, const std::string& oldPassword, const std::string& newPassword, const error_type &er)
            : base_class(sender, er), oldPassword_(oldPassword), newPassword_(newPassword)
        {

        }

        ChangePasswordResponce::ChangePasswordResponce(const base_class &request)
            : base_class(request)
        {
        }

        ChangeMaxConnectionRequest::ChangeMaxConnectionRequest(initiator_type sender, int maxConnection, const error_type &er)
            : base_class(sender, er), maxConnection_(maxConnection)
        {

        }

        ChangeMaxConnectionResponce::ChangeMaxConnectionResponce(const base_class &request)
            : base_class(request)
        {
        }

        ProcessConfigArgsInfoRequest::ProcessConfigArgsInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        ProcessConfigArgsInfoResponce::ProcessConfigArgsInfoResponce(const base_class& request)
            : base_class(request)
        {
        }

        DiscoveryInfoRequest::DiscoveryInfoRequest(initiator_type sender, const error_type& er)
            : base_class(sender, er)
        {

        }

        DiscoveryInfoResponce::DiscoveryInfoResponce(const base_class& request)
            : base_class(request)
        {
        }

        EnterModeInfo::EnterModeInfo(initiator_type sender, ConnectionMode mode, const error_type& er)
            : base_class(sender, er), mode_(mode)
        {

        }

        LeaveModeInfo::LeaveModeInfo(initiator_type sender, ConnectionMode mode, const error_type& er)
            : base_class(sender, er), mode_(mode)
        {

        }

        CommandRootCreatedInfo::CommandRootCreatedInfo(initiator_type sender, FastoObjectIPtr root, const error_type &er)
            : base_class(sender, er), root_(root)
        {

        }

        CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender, FastoObjectIPtr root, const error_type &er)
            : base_class(sender, er), root_(root)
        {

        }

        CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender, common::time64_t timest, FastoObjectIPtr root, const error_type &er)
            : base_class(sender, timest, er), root_(root)
        {

        }

        DisConnectInfoRequest::DisConnectInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        DisConnectInfoResponce::DisConnectInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        ExecuteInfoRequest::ExecuteInfoRequest(initiator_type sender, const std::string &text, const std::vector<std::string>& args, const error_type &er)
            : base_class(sender, er), text_(text), args_(args)
        {

        }

        LoadDatabasesInfoRequest::LoadDatabasesInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        LoadDatabasesInfoResponce::LoadDatabasesInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        LoadDatabaseContentRequest::LoadDatabaseContentRequest(initiator_type sender, DataBaseInfoSPtr inf, const std::string& pattern, uint32_t countKeys,
                                                               uint32_t cursor, const error_type &er)
            : base_class(sender, er), inf_(inf), pattern_(pattern), countKeys_(countKeys), cursorIn_(cursor)
        {

        }

        LoadDatabaseContentResponce::LoadDatabaseContentResponce(const base_class &request)
            : base_class(request)
        {
        }

        SetDefaultDatabaseRequest::SetDefaultDatabaseRequest(initiator_type sender, DataBaseInfoSPtr inf, const error_type &er)
            : base_class(sender, er), inf_(inf)
        {

        }

        SetDefaultDatabaseResponce::SetDefaultDatabaseResponce(const base_class &request)
            : base_class(request)
        {
        }

        ServerInfoRequest::ServerInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        ServerInfoResponce::ServerInfoResponce(const base_class &request)
            : base_class(request), info_()
        {
        }

        ServerInfoSPtr ServerInfoResponce::info() const
        {
            return info_;
        }

        void ServerInfoResponce::setInfo(ServerInfoSPtr inf)
        {
            info_ = inf;
        }

        ServerInfoResponce::~ServerInfoResponce()
        {

        }

        ServerInfoHistoryRequest::ServerInfoHistoryRequest(initiator_type sender, const error_type& er)
            : base_class(sender, er)
        {

        }

        ServerInfoHistoryResponce::ServerInfoHistoryResponce(const base_class &request)
            : base_class(request)
        {
        }

        ServerInfoHistoryResponce::infos_container_type ServerInfoHistoryResponce::infos() const
        {
            return infos_;
        }

        void ServerInfoHistoryResponce::setInfos(const infos_container_type& inf)
        {
            infos_ = inf;
        }

        ServerPropertyInfoRequest::ServerPropertyInfoRequest(initiator_type sender, const error_type &er)
            : base_class(sender, er)
        {

        }

        ServerPropertyInfoResponce::ServerPropertyInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        ChangeServerPropertyInfoRequest::ChangeServerPropertyInfoRequest(initiator_type sender, const PropertyType &pt, const error_type &er)
            : base_class(sender, er), newItem_(pt)
        {

        }

        ChangeServerPropertyInfoResponce::ChangeServerPropertyInfoResponce(const base_class &request)
            : base_class(request)
        {
        }

        CommandRequest::CommandRequest(initiator_type sender, DataBaseInfoSPtr inf, CommandKeySPtr cmd, const error_type &er)
            : base_class(sender, er), inf_(inf), cmd_(cmd)
        {

        }

        CommandResponce::CommandResponce(const base_class& request)
            : base_class(request)
        {
        }

        ProgressInfoResponce::ProgressInfoResponce(uint8_t pr)
            : progress_(pr)
        {

        }
    }
}
