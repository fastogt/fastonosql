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

#include <common/net/types.h>  // for HostAndPort

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/driver/idriver.h"                            // for IDriver

namespace fastonosql {
namespace proxy {

class IDriverRemote : public IDriver {
  Q_OBJECT

 public:
  common::net::HostAndPort GetHost() const;

  core::translator_t GetTranslator() const override = 0;

  bool IsInterrupted() const override = 0;
  void SetInterrupted(bool interrupted) override = 0;

  bool IsConnected() const override = 0;
  bool IsAuthenticated() const override = 0;

 protected:
  explicit IDriverRemote(IConnectionSettingsBaseSPtr settings);
};

}  // namespace proxy
}  // namespace fastonosql
