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

#pragma once

#include <QFont>  // for QFont

#include <common/patterns/singleton_pattern.h>  // for LazySingleton
#include <common/value.h>                       // for Value, Value::Type

#include <fastonosql/core/connection_types.h>  // for ConnectionType, etc

class QIcon;
class QString;

namespace fastonosql {
namespace gui {

class GuiFactory : public common::patterns::LazySingleton<GuiFactory> {
 public:
  friend class common::patterns::LazySingleton<GuiFactory>;

  const QIcon& directoryIcon() const;

  const QIcon& connectDBIcon() const;
  const QIcon& openIcon() const;
  const QIcon& logoIcon() const;
  const QIcon& mainWindowIcon() const;
  const QIcon& connectIcon() const;
  const QIcon& disConnectIcon() const;
  const QIcon& serverIcon() const;
  const QIcon& addIcon() const;
  const QIcon& removeIcon() const;
  const QIcon& editIcon() const;
  const QIcon& cloneIcon() const;
  const QIcon& messageBoxInformationIcon() const;
  const QIcon& messageBoxQuestionIcon() const;
  const QIcon& validateIcon() const;
  const QIcon& executeIcon() const;
  const QIcon& helpIcon() const;
  const QIcon& timeIcon() const;
  const QIcon& stopIcon() const;
  const QIcon& databaseIcon() const;
  const QIcon& moduleIcon() const;
  const QIcon& keyIcon() const;
  const QIcon& keyTTLIcon() const;

  const QIcon& icon(core::ConnectionType type) const;
  const QIcon& modeIcon(core::ConnectionMode mode) const;
  const QIcon& icon(common::Value::Type type) const;

  const QIcon& importIcon() const;
  const QIcon& exportIcon() const;

  const QIcon& loadIcon() const;
  const QIcon& clusterIcon() const;
  const QIcon& sentinelIcon() const;
  const QIcon& saveIcon() const;
  const QIcon& saveAsIcon() const;
  const QIcon& textIcon() const;
  const QIcon& tableIcon() const;
  const QIcon& treeIcon() const;
  const QIcon& loggingIcon() const;
  const QIcon& discoveryIcon() const;
  const QIcon& channelIcon() const;
  const QIcon& commandIcon() const;
  const QIcon& encodeDecodeIcon() const;
  const QIcon& preferencesIcon() const;

  const QIcon& leftIcon() const;
  const QIcon& rightIcon() const;

  const QIcon& close16Icon() const;
  const QIcon& search16Icon() const;
  const QIcon& commandIcon(core::ConnectionType type) const;

  const QIcon& successIcon() const;
  const QIcon& failIcon() const;
  const QIcon& unknownIcon() const;
  const QIcon& infoIcon() const;

  const QIcon& welcomeTabIcon() const;
  const QIcon& githubIcon() const;
  const QIcon& twitterIcon() const;
  const QIcon& facebookIcon() const;
  const QIcon& instagramIcon() const;
  const QIcon& youtubeIcon() const;
  const QIcon& emailIcon() const;
  const QIcon& homePageIcon() const;

  QFont font() const;
  const QString& pathToLoadingGif() const;
  QString pathToIndividualBuilds() const;
  QString pathToConnectGif() const;
  QString pathToWorkflowGif() const;

 private:
  GuiFactory();
};

}  // namespace gui
}  // namespace fastonosql
