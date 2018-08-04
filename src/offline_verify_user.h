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

#include "iverify_user.h"

namespace fastonosql {

class OfflineVerifyUser : public IVerifyUser {
 public:
  typedef IVerifyUser base_class;
  OfflineVerifyUser(const QString& login, const QString& password, QObject* parent = Q_NULLPTR);

 private:
  virtual common::Error startVerificationImpl(const std::string& login,
                                              const std::string& hexed_password,
                                              proxy::UserInfo* uinf) override WARN_UNUSED_RESULT;
};

}  // namespace fastonosql
