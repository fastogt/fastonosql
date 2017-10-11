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

#include <string>  // for string

namespace fastonosql {
namespace core {

class NDbPSChannel {
 public:
  NDbPSChannel();
  NDbPSChannel(const std::string& name, size_t nos);

  std::string GetName() const;
  void SetName(const std::string& name);

  size_t GetNumberOfSubscribers() const;
  void SetNumberOfSubscribers(size_t nos);

 private:
  std::string name_;
  size_t number_of_subscribers_;
};

}  // namespace core
}  // namespace fastonosql
