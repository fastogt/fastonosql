/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "translations/global.h"

#include <QObject>

namespace fastonosql {
namespace translations {

const QString trfilterForScripts = QObject::tr("Text Files (*.txt);; All Files (*)");
const QString trfilterForAll = QObject::tr("All Files (*)");
const QString trfilterForRdb = QObject::tr("Redis database files (*.rdb)");

const QString trBasic = QObject::tr("Basic");
const QString trAdvanced = QObject::tr("Advanced");
const QString trLoad = QObject::tr("Load");
const QString trRepeat = QObject::tr("Repeat");
const QString trInterval = QObject::tr("Interval");
const QString trAddConnection = QObject::tr("Add connnection");
const QString trAddClusterConnection = QObject::tr("Add cluster connnection");
const QString trAddSentinelConnection = QObject::tr("Add sentinel connnection");
const QString trRemoveConnection = QObject::tr("Remove connnection");
const QString trCloneConnection = QObject::tr("Clone connnection");
const QString trEditConnection = QObject::tr("Edit connection");
const QString trOpen = QObject::tr("Open");
const QString trMenu = QObject::tr("Menu");
const QString trClose = QObject::tr("Close");
const QString trSave = QObject::tr("Save");
const QString trSaveChanges = QObject::tr("Save changes");
const QString trExit = QObject::tr("Exit");
const QString trFilter = QObject::tr("Filter");
const QString trViews = QObject::tr("Views");
const QString trHistory = QObject::tr("History");
const QString trConsole = QObject::tr("Console");
const QString trOutput = QObject::tr("Output");
const QString trClearHistory = QObject::tr("Clear history");
const QString trFile = QObject::tr("File");
const QString trDirectory = QObject::tr("Directory");
const QString trInfo = QObject::tr("Info");
const QString trTools = QObject::tr("Tools");
const QString trLoadFromFile = QObject::tr("Load from file");
const QString trImportSettings = QObject::tr("Import settings");
const QString trExportSettings = QObject::tr("Export settings");
const QString trProperty = QObject::tr("Property");
const QString trStop = QObject::tr("Stop");
const QString trError = QObject::tr("Error");
const QString trNewConnection = QObject::tr("New Connection");
const QString trClustersAvailibleOnlyInProVersion = QObject::tr("Clusters availible only in PRO version!");
const QString trSentinelsAvailibleOnlyInProVersion = QObject::tr("Sentinels availible only in PRO version!");
const QString trProLimitations = QObject::tr("PRO version limitations");
const QString trLogs = QObject::tr("Logs");
const QString trKey = QObject::tr("Key");
const QString trValue = QObject::tr("Value");
const QString trString = QObject::tr("String");
const QString trAction = QObject::tr("Action");
const QString trGetValue = QObject::tr("Get value");
const QString trGetType = QObject::tr("Get type");
const QString trEditValue = QObject::tr("Edit value");
const QString trMember = QObject::tr("Member");
const QString trScore = QObject::tr("Score");
const QString trField = QObject::tr("Field");
const QString trRename = QObject::tr("Rename");
const QString trRemove = QObject::tr("Remove");
const QString trWatch = QObject::tr("Watch");
const QString trOptions = QObject::tr("Options");
const QString trWindow = QObject::tr("Window");
const QString trHelp = QObject::tr("Help");
const QString trConnect = QObject::tr("Connect");
const QString trWorkflow = QObject::tr("Workflow");
const QString trIndividualBuilds = QObject::tr("Individual builds");
const QString trDisconnect = QObject::tr("Disconnect");
const QString trValidate = QObject::tr("Validate");
const QString trExecute = QObject::tr("Execute");
const QString trPreferences = QObject::tr("Preferences");
const QString trConnections = QObject::tr("Connections");
const QString trRemoveConnectionTemplate_1S = QObject::tr("Really remove \"%1\" connection?");
const QString trRemoveClusterTemplate_1S = QObject::tr("Really remove \"%1\" cluster?");
const QString trRemoveSentinelTemplate_1S = QObject::tr("Really remove \"%1\" sentinel?");
const QString trTimeTemplate_1S = QObject::tr("Time execute msec: %1");
const QString trConnectionStatusTemplate_1S = QObject::tr("Connection state: %1");
const QString trSuccess = QObject::tr("Success");
const QString trReload = QObject::tr("Reload");
const QString trDuplicate = QObject::tr("Duplicate");
const QString trType = QObject::tr("Type");
const QString trState = QObject::tr("State");
const QString trCommands = QObject::tr("Commands");
const QString trCommand = QObject::tr("Command");
const QString trResult = QObject::tr("Result");
const QString trCalculate = QObject::tr("Calculate");
const QString trName = QObject::tr("Name");
const QString trNumberOfSubscribers = QObject::tr("Number of subscribers");
const QString trAddress = QObject::tr("Address");
const QString trPassword = QObject::tr("Password");
const QString trAuthentication = QObject::tr("Authentication");
const QString trAskPassword = QObject::tr("Ask password");
const QString trTrial = QObject::tr("Trial");
const QString trLogin = QObject::tr("Login");
const QString trFirstName = QObject::tr("First Name");
const QString trLastName = QObject::tr("Last Name");
const QString trHide = QObject::tr("Hide");
const QString trShow = QObject::tr("Show");
const QString trFolder = QObject::tr("Folder");
const QString trLoggingEnabled = QObject::tr("Logging enabled");

const QString trCannotConvertPattern_1S = QObject::tr("Cannot convert to '%1' format!");
const QString trSearch = QObject::tr("Search");
const QString trBackup = QObject::tr("Backup");
const QString trRestore = QObject::tr("Restore");
const QString trNext = QObject::tr("Next");
const QString trPrevious = QObject::tr("Previous");
const QString trTryToConnect = QObject::tr("Try to connect");
const QString trAddItem = QObject::tr("Add item");
const QString trRemoveItem = QObject::tr("Remove item");
const QString trInvalidInput = QObject::tr("Invalid input");

const QString trMatchCase = QObject::tr("Match case");
const QString trSaveAs = QObject::tr("Save as");
const QString trClearAll = QObject::tr("Clear all");
const QString trClearMenu = QObject::tr("Clear menu");
const QString trPrivateKey = QObject::tr("Private key");
const QString trPublicPrivateKey = QObject::tr("Public/Private key");
const QString trOpenConsole = QObject::tr("Open console");
const QString trSetDefault = QObject::tr("Set default");
const QString trSetAsStartNode = QObject::tr("Set as start node");
const QString trLoadDataBases = QObject::tr("Load databases");
const QString trCreateDatabase = QObject::tr("Create database");
const QString trRemoveDatabase = QObject::tr("Remove database");
const QString trExpTree = QObject::tr("Explorer tree");
const QString trRecentConnections = QObject::tr("Recent connections");
const QString trCheckVersion = QObject::tr("Check version");
const QString trNewTab = QObject::tr("New tab");
const QString trNextTab = QObject::tr("Next tab");
const QString trPrevTab = QObject::tr("Prev tab");
const QString trCloseTab = QObject::tr("Close tab");

const QString trFullScreen = QObject::tr("Full screen");
const QString trExecuteWithArgs = QObject::tr("Execute with args");
const QString trEdit = QObject::tr("Edit");
const QString trReportBug = QObject::tr("Report bug");
const QString trHowToUse = QObject::tr("How to use");
const QString trCheckUpdate = QObject::tr("Check for updates");
const QString trSendStatistic = QObject::tr("Send statistic");
const QString trCloseOtherTabs = QObject::tr("Close other tabs");

const QString trLoadAndExecuteFile = QObject::tr("Load and execute file");
const QString trLoadContOfDataBases = QObject::tr("Load content of database");
const QString trRemoveAllKeys = QObject::tr("Remove all keys");
const QString trRemoveBranch = QObject::tr("Remove branch");
const QString trRenameBranch = QObject::tr("Rename branch");
const QString trRemoveKey = QObject::tr("Remove key");
const QString trRenameKey = QObject::tr("Rename key");
const QString trCreateKey = QObject::tr("Create key");
const QString trViewKeysDialog = QObject::tr("View keys dialog");
const QString trPubSubDialog = QObject::tr("Publish/Subscribe dialog");
const QString trPublish = QObject::tr("Publish");
const QString trEncodeDecode = QObject::tr("Encode/Decode");
const QString trEncode = QObject::tr("Encode");
const QString trDecode = QObject::tr("Decode");
const QString trConnectionDiagnostic = QObject::tr("Connection diagnostic");
const QString trConnectionDiscovery = QObject::tr("Connection discovery");
const QString trKeyCountOnThePage = QObject::tr("Key count on the page");
const QString trInvalidDbType = QObject::tr("Invalid database type!");

}  // namespace translations
}  // namespace fastonosql
