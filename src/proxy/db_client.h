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

#include <string>

#include <common/net/types.h>

// id=139 addr=127.0.0.1:38396 fd=8 name= age=26 idle=0 flags=N db=0 sub=0 psub=0 multi=-1
// qbuf=26 qbuf-free=32742 obl=0 oll=0 omem=0 events=r cmd=client

namespace fastonosql {
namespace proxy {

class NDbClient {
 public:
  typedef int id_t;
  typedef common::net::HostAndPort addr_t;
  typedef descriptor_t fd_t;
  typedef std::string name_t;
  typedef int age_t;
  typedef int idle_t;
  typedef std::string flags_t;
  typedef int db_t;
  typedef int sub_t;
  typedef int psub_t;
  typedef int multi_t;
  typedef int qbuf_t;
  typedef int qbuf_free_t;
  typedef int odl_t;
  typedef int oll_t;
  typedef int omem_t;
  typedef std::string events_t;
  typedef std::string cmd_t;

  NDbClient();
  explicit NDbClient(const std::string& text);

  bool IsValid() const;

  void SetId(id_t iden);
  id_t GetId() const;

  void SetAddr(const addr_t& addr);
  addr_t GetAddr() const;

  void SetFd(fd_t fd);
  fd_t GetFd() const;

  void SetAge(age_t age);
  age_t GetAge() const;

  void SetName(const name_t& name);
  name_t GetName() const;

  void SetIdle(idle_t idle);
  age_t GetIdle() const;

  void SetFlags(const flags_t& flags);
  flags_t GetFlags() const;

  void SetDb(db_t db);
  db_t GetDb() const;

  void SetSub(sub_t sub);
  sub_t GetSub() const;

  void SetPSub(psub_t psub);
  psub_t GetPSub() const;

  void SetMulti(multi_t multi);
  multi_t GetMulti() const;

  void SetQbuf(qbuf_t qbuf);
  qbuf_t GetQbuf() const;

  void SetQbufFree(qbuf_free_t qbuf_free);
  qbuf_free_t GetQbufFree() const;

  void SetOdl(odl_t odl);
  odl_t GetOdl() const;

  void SetOll(oll_t oll);
  oll_t GetOll() const;

  void SetOmem(omem_t omem);
  omem_t GetOmem() const;

  void SetEvents(const events_t& events);
  events_t GetEvents() const;

  void SetCmd(const cmd_t& cmd);
  cmd_t GetCmd() const;

 private:
  id_t id_;
  addr_t addr_;
  fd_t fd_;
  name_t name_;
  age_t age_;
  idle_t idle_;
  flags_t flags_;
  db_t db_;
  sub_t sub_;
  psub_t psub_;
  multi_t multi_;
  qbuf_t qbuf_;
  qbuf_free_t qbuf_free_;
  odl_t odl_;
  oll_t oll_;
  omem_t omem_;
  events_t events_;
  cmd_t cmd_;
};

}  // namespace proxy
}  // namespace fastonosql
