/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#pragma once

#include <QFont>  // for QFont

#include <common/patterns/singleton_pattern.h>  // for LazySingleton
#include <common/value.h>                       // for Value, Value::Type

#include "core/connection_types.h"  // for connectionTypes, etc

class QIcon;
class QString;

namespace fastonosql {
namespace gui {

class GuiFactory : public common::patterns::LazySingleton<GuiFactory> {
 public:
  friend class common::patterns::LazySingleton<GuiFactory>;

  const QIcon& GetDirectoryIcon() const;

  const QIcon& GetOpenIcon() const;
  const QIcon& GetLogoIcon() const;
  const QIcon& GetMainWindowIcon() const;
  const QIcon& GetConnectIcon() const;
  const QIcon& GetDisConnectIcon() const;
  const QIcon& GetServerIcon() const;
  const QIcon& GetAddIcon() const;
  const QIcon& GetRemoveIcon() const;
  const QIcon& GetEditIcon() const;
  const QIcon& GetMessageBoxInformationIcon() const;
  const QIcon& GetMessageBoxQuestionIcon() const;
  const QIcon& GetValidateIcon() const;
  const QIcon& GetExecuteIcon() const;
  const QIcon& GetHelpIcon() const;
  const QIcon& GetTimeIcon() const;
  const QIcon& stopIcon() const;
  const QIcon& GetDatabaseIcon() const;
  const QIcon& GetModuleIcon() const;
  const QIcon& GetKeyIcon() const;
  const QIcon& GetKeyTTLIcon() const;

  const QIcon& GetIcon(core::connectionTypes type) const;
  const QIcon& GetModeIcon(core::ConnectionMode mode) const;
  const QIcon& GetIcon(common::Value::Type type) const;

  const QIcon& GetImportIcon() const;
  const QIcon& GetExportIcon() const;

  const QIcon& GetLoadIcon() const;
  const QIcon& GetClusterIcon() const;
  const QIcon& GetSentinelIcon() const;
  const QIcon& GetSaveIcon() const;
  const QIcon& GetSaveAsIcon() const;
  const QIcon& GetTextIcon() const;
  const QIcon& GetTableIcon() const;
  const QIcon& GetTreeIcon() const;
  const QIcon& GetLoggingIcon() const;
  const QIcon& GetDiscoveryIcon() const;
  const QIcon& GetChannelIcon() const;
  const QIcon& GetCommandIcon() const;
  const QIcon& GetEncodeDecodeIcon() const;
  const QIcon& GetPreferencesIcon() const;

  const QIcon& GetLeftIcon() const;
  const QIcon& GetRightIcon() const;

  const QIcon& GetClose16Icon() const;
  const QIcon& GetSearch16Icon() const;
  const QIcon& GetCommandIcon(core::connectionTypes type) const;

  const QIcon& GetSuccessIcon() const;
  const QIcon& GetFailIcon() const;
  const QIcon& GetUnknownIcon() const;

  QFont GetFont() const;
  const QString& GetPathToLoadingGif() const;

 private:
  GuiFactory();
  const QIcon& redisConnectionIcon() const;
  const QIcon& memcachedConnectionIcon() const;
  const QIcon& ssdbConnectionIcon() const;
  const QIcon& leveldbConnectionIcon() const;
  const QIcon& rocksdbConnectionIcon() const;
  const QIcon& unqliteConnectionIcon() const;
  const QIcon& lmdbConnectionIcon() const;
  const QIcon& upscaledbConnectionIcon() const;
  const QIcon& forestdbConnectionIcon() const;
};

}  // namespace gui
}  // namespace fastonosql
