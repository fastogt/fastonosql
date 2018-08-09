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

#include <QObject>

#include <common/error.h>

#include "proxy/user_info.h"

namespace fastonosql {

class IVerifyUser : public QObject {
  Q_OBJECT
 public:
  IVerifyUser(const QString& login,
              const QString& password,
              proxy::UserInfo::BuildStrategy build_strategy,
              QObject* parent = Q_NULLPTR);
  virtual ~IVerifyUser();

 Q_SIGNALS:
  void verifyUserResult(common::Error err, const proxy::UserInfo& user);

 public Q_SLOTS:
  void routine();

 protected:
  common::Error startVerification(const QString& login,
                                  const QString& password,
                                  proxy::UserInfo::BuildStrategy strategy,
                                  proxy::UserInfo* uinf) WARN_UNUSED_RESULT;

 private:
  virtual common::Error startVerificationImpl(const std::string& login,
                                              const std::string& hexed_password,
                                              proxy::UserInfo::BuildStrategy strategy,
                                              proxy::UserInfo* uinf) WARN_UNUSED_RESULT = 0;

  const QString login_;
  const QString password_;
  const proxy::UserInfo::BuildStrategy build_strategy_;
};

}  // namespace fastonosql
