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

#define REDIS_MODULE_COMMAND_GENERATE(MODULE, COMMAND) MODULE "." COMMAND

#define REDIS_GRAPH_MODULE "GRAPH"
#define REDIS_GRAPH_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_GRAPH_MODULE, COMMAND)

#define REDIS_SEARCH_MODULE "FT"
#define REDIS_SEARCH_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_SEARCH_MODULE, COMMAND)

#define REDIS_JSON_MODULE "JSON"
#define REDIS_JSON_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_JSON_MODULE, COMMAND)

#define REDIS_NR_MODULE "NR"
#define REDIS_NR_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_NR_MODULE, COMMAND)

#define REDIS_BLOOM_MODULE "BF"
#define REDIS_BLOOM_MODULE_COMMAND(COMMAND) REDIS_MODULE_COMMAND_GENERATE(REDIS_BLOOM_MODULE, COMMAND)

namespace fastonosql {
namespace core {
namespace redis {}  // namespace redis
}  // namespace core
}  // namespace fastonosql
