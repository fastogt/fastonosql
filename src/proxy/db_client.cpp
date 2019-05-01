/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db_client.h"

#include <common/convert2string.h>

#define CLIENT_ID "id"
#define CLIENT_ADDR "addr"
#define CLIENT_FD "fd"
#define CLIENT_NAME "name"
#define CLIENT_AGE "age"
#define CLIENT_IDLE "idle"
#define CLIENT_FLAGS "flags"
#define CLIENT_DB "db"
#define CLIENT_SUB "sub"
#define CLIENT_PSUB "psub"
#define CLIENT_MULTI "multi"
#define CLIENT_QBUF "qbuf"
#define CLIENT_QBUF_FREE "qbuf-free"
#define CLIENT_ODL "obl"
#define CLIENT_OLL "oll"
#define CLIENT_OMEM "omem"
#define CLIENT_EVENTS "events"
#define CLIENT_CMD "cmd"

#define CLIENT_FIELDS_DELEMITER " "
#define CLIENT_FIELD_VALUE_DELEMITER "="

#define INVALID_ID -1

namespace fastonosql {
namespace proxy {

NDbClient::NDbClient()
    : id_(INVALID_ID),
      addr_(),
      fd_(0),
      name_(),
      age_(0),
      idle_(0),
      flags_(),
      db_(0),
      sub_(0),
      psub_(0),
      multi_(0),
      qbuf_(0),
      qbuf_free_(0),
      odl_(0),
      oll_(0),
      omem_(0),
      events_(),
      cmd_() {}

NDbClient::NDbClient(const std::string& text) : NDbClient() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = text.find(CLIENT_FIELDS_DELEMITER, start)) != std::string::npos) {
    std::string line = text.substr(start, pos - start);
    size_t delem = line.find_first_of(CLIENT_FIELD_VALUE_DELEMITER);
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == CLIENT_ID) {
      id_t iden;
      if (common::ConvertFromString(value, &iden)) {
        id_ = iden;
      }
    } else if (field == CLIENT_ADDR) {
      addr_t hs;
      if (common::ConvertFromString(value, &hs)) {
        addr_ = hs;
      }
    } else if (field == CLIENT_FD) {
      fd_t fd;
      if (common::ConvertFromString(value, &fd)) {
        fd_ = fd;
      }
    } else if (field == CLIENT_NAME) {
      name_ = value;
    } else if (field == CLIENT_AGE) {
      age_t age;
      if (common::ConvertFromString(value, &age)) {
        age_ = age;
      }
    } else if (field == CLIENT_IDLE) {
      idle_t idle;
      if (common::ConvertFromString(value, &idle)) {
        idle_ = idle;
      }
    } else if (field == CLIENT_FLAGS) {
      flags_ = value;
    } else if (field == CLIENT_DB) {
      db_t db;
      if (common::ConvertFromString(value, &db)) {
        db_ = db;
      }
    } else if (field == CLIENT_SUB) {
      sub_t ps;
      if (common::ConvertFromString(value, &ps)) {
        sub_ = ps;
      }
    } else if (field == CLIENT_PSUB) {
      psub_t ps;
      if (common::ConvertFromString(value, &ps)) {
        psub_ = ps;
      }
    } else if (field == CLIENT_MULTI) {
      multi_t multi;
      if (common::ConvertFromString(value, &multi)) {
        multi_ = multi;
      }
    } else if (field == CLIENT_QBUF) {
      qbuf_t qbuf;
      if (common::ConvertFromString(value, &qbuf)) {
        qbuf_ = qbuf;
      }
    } else if (field == CLIENT_QBUF_FREE) {
      qbuf_free_t qbuf_free;
      if (common::ConvertFromString(value, &qbuf_free)) {
        qbuf_free_ = qbuf_free;
      }
    } else if (field == CLIENT_ODL) {
      odl_t odl;
      if (common::ConvertFromString(value, &odl)) {
        odl_ = odl;
      }
    } else if (field == CLIENT_OLL) {
      oll_t oll;
      if (common::ConvertFromString(value, &oll)) {
        oll_ = oll;
      }
    } else if (field == CLIENT_OMEM) {
      omem_t omem;
      if (common::ConvertFromString(value, &omem)) {
        omem_ = omem;
      }
    } else if (field == CLIENT_EVENTS) {
      events_ = value;
    } else if (field == CLIENT_CMD) {
      cmd_ = value;
    }
    start = pos + SIZEOFMASS(CLIENT_FIELDS_DELEMITER) - 1;
  }
}

bool NDbClient::IsValid() const {
  return id_ != INVALID_ID;
}

void NDbClient::SetId(id_t iden) {
  id_ = iden;
}

NDbClient::id_t NDbClient::GetId() const {
  return id_;
}

void NDbClient::SetAddr(const addr_t& addr) {
  addr_ = addr;
}

NDbClient::addr_t NDbClient::GetAddr() const {
  return addr_;
}

void NDbClient::SetFd(fd_t fd) {
  fd_ = fd;
}

NDbClient::fd_t NDbClient::GetFd() const {
  return fd_;
}

void NDbClient::SetAge(age_t age) {
  age_ = age;
}

NDbClient::age_t NDbClient::GetAge() const {
  return age_;
}

void NDbClient::SetName(const name_t& name) {
  name_ = name;
}

NDbClient::name_t NDbClient::GetName() const {
  return name_;
}

void NDbClient::SetIdle(idle_t idle) {
  idle_ = idle;
}

NDbClient::age_t NDbClient::GetIdle() const {
  return idle_;
}

void NDbClient::SetFlags(const flags_t& flags) {
  flags_ = flags;
}

NDbClient::flags_t NDbClient::GetFlags() const {
  return flags_;
}

void NDbClient::SetDb(db_t db) {
  db_ = db;
}

NDbClient::db_t NDbClient::GetDb() const {
  return db_;
}

void NDbClient::SetSub(sub_t sub) {
  sub_ = sub;
}

NDbClient::sub_t NDbClient::GetSub() const {
  return sub_;
}

void NDbClient::SetPSub(psub_t psub) {
  psub_ = psub;
}

NDbClient::psub_t NDbClient::GetPSub() const {
  return psub_;
}

void NDbClient::SetMulti(multi_t multi) {
  multi_ = multi;
}

NDbClient::multi_t NDbClient::GetMulti() const {
  return multi_;
}

void NDbClient::SetQbuf(qbuf_t qbuf) {
  qbuf_ = qbuf;
}

NDbClient::qbuf_t NDbClient::GetQbuf() const {
  return qbuf_;
}

void NDbClient::SetQbufFree(qbuf_free_t qbuf_free) {
  qbuf_free_ = qbuf_free;
}

NDbClient::qbuf_free_t NDbClient::GetQbufFree() const {
  return qbuf_;
}

void NDbClient::SetOdl(odl_t odl) {
  odl_ = odl;
}

NDbClient::odl_t NDbClient::GetOdl() const {
  return odl_;
}

void NDbClient::SetOll(oll_t oll) {
  oll_ = oll;
}

NDbClient::oll_t NDbClient::GetOll() const {
  return oll_;
}

void NDbClient::SetOmem(omem_t omem) {
  omem_ = omem;
}

NDbClient::omem_t NDbClient::GetOmem() const {
  return omem_;
}

void NDbClient::SetEvents(const events_t& events) {
  events_ = events;
}

NDbClient::events_t NDbClient::GetEvents() const {
  return events_;
}

void NDbClient::SetCmd(const cmd_t& cmd) {
  cmd_ = cmd;
}

NDbClient::cmd_t NDbClient::GetCmd() const {
  return cmd_;
}

}  // namespace proxy
}  // namespace fastonosql
