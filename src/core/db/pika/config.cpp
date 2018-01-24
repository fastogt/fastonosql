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

#include "core/db/pika/config.h"

#define DEFAULT_PIKA_SERVER_PORT 9211

namespace fastonosql {
namespace core {
namespace pika {

Config::Config() : base_class(common::net::HostAndPort::CreateLocalHost(DEFAULT_PIKA_SERVER_PORT)) {}

RConfig::RConfig(const Config& config, const SSHInfo& sinfo) : Config(config), ssh_info(sinfo) {}

}  // namespace pika
}  // namespace core
}  // namespace fastonosql
