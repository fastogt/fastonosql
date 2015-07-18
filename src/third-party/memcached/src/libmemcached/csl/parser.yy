/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Libmemcached library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

%{

#include <libmemcached/csl/common.h>

class Context;

%}

%error-verbose
%debug
%defines
%expect 0
%output "libmemcached/csl/parser.cc"
%defines "libmemcached/csl/parser.h"
%lex-param { yyscan_t *scanner }
%name-prefix="config_"
%parse-param { class Context *context }
%parse-param { yyscan_t *scanner }
%pure-parser
%require "2.5"
%start begin
%verbose

%{

#include <libmemcached/options.hpp>

#include <libmemcached/csl/context.h>
#include <libmemcached/csl/symbol.h>
#include <libmemcached/csl/scanner.h>

#ifndef __INTEL_COMPILER
# pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifndef __INTEL_COMPILER
# ifndef __clang__
#  pragma GCC diagnostic ignored "-Wlogical-op"
#  pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
# endif
#endif

int conf_lex(YYSTYPE* lvalp, void* scanner);

#define select_yychar(__context) yychar == UNKNOWN ? ( (__context)->previous_token == END ? UNKNOWN : (__context)->previous_token ) : yychar   

#define stryytname(__yytokentype) ((__yytokentype) <  YYNTOKENS ) ? yytname[(__yytokentype)] : ""

#define parser_abort(__context, __error_message) do { (__context)->abort((__error_message), yytokentype(select_yychar(__context)), stryytname(YYTRANSLATE(select_yychar(__context)))); YYABORT; } while (0) 

// This is bison calling error.
inline void __config_error(Context *context, yyscan_t *scanner, const char *error, int last_token, const char *last_token_str)
{
  if (not context->end())
  {
    context->error(error, yytokentype(last_token), last_token_str);
  }
  else
  {
    context->error(error, yytokentype(last_token), last_token_str);
  }
}

#define config_error(__context, __scanner, __error_msg) do { __config_error((__context), (__scanner), (__error_msg), select_yychar(__context), stryytname(YYTRANSLATE(select_yychar(__context)))); } while (0)


%}

%token COMMENT
%token END
%token CSL_ERROR
%token RESET
%token PARSER_DEBUG
%token INCLUDE
%token CONFIGURE_FILE
%token EMPTY_LINE
%token SERVER
%token CSL_SOCKET
%token SERVERS
%token SERVERS_OPTION
%token UNKNOWN_OPTION
%token UNKNOWN

/* All behavior options */
%token BINARY_PROTOCOL
%token BUFFER_REQUESTS
%token CONNECT_TIMEOUT
%token DISTRIBUTION
%token HASH
%token HASH_WITH_NAMESPACE
%token IO_BYTES_WATERMARK
%token IO_KEY_PREFETCH
%token IO_MSG_WATERMARK
%token KETAMA_HASH
%token KETAMA_WEIGHTED
%token NOREPLY
%token NUMBER_OF_REPLICAS
%token POLL_TIMEOUT
%token RANDOMIZE_REPLICA_READ
%token RCV_TIMEOUT
%token REMOVE_FAILED_SERVERS
%token RETRY_TIMEOUT
%token SND_TIMEOUT
%token SOCKET_RECV_SIZE
%token SOCKET_SEND_SIZE
%token SORT_HOSTS
%token SUPPORT_CAS
%token USER_DATA
%token USE_UDP
%token VERIFY_KEY
%token _TCP_KEEPALIVE
%token _TCP_KEEPIDLE
%token _TCP_NODELAY
%token FETCH_VERSION

/* Callbacks */
%token NAMESPACE

/* Pool */
%token POOL_MIN
%token POOL_MAX

/* Hash types */
%token MD5
%token CRC
%token FNV1_64
%token FNV1A_64
%token FNV1_32
%token FNV1A_32
%token HSIEH
%token MURMUR
%token JENKINS

/* Distributions */
%token CONSISTENT
%token MODULA
%token RANDOM

/* Boolean values */
%token <boolean> CSL_TRUE
%token <boolean> CSL_FALSE

%nonassoc ','
%nonassoc '='

%token <number> CSL_FLOAT
%token <number> NUMBER
%token <number> PORT
%token <number> WEIGHT_START
%token <server> IPADDRESS
%token <server> HOSTNAME
%token <string> STRING
%token <string> QUOTED_STRING
%token <string> FILE_PATH

%type <behavior> behavior_boolean
%type <behavior> behavior_number
%type <distribution> distribution
%type <hash> hash
%type <number> optional_port
%type <number> optional_weight
%type <string> string

%%

begin:
          statement
        | begin ' ' statement
        ;

statement:
         expression
          { }
        | COMMENT
          { }
        | EMPTY_LINE
          { }
        | END
          {
            context->set_end();
            YYACCEPT;
          }
        | CSL_ERROR
          {
            context->rc= MEMCACHED_PARSE_USER_ERROR;
            parser_abort(context, "ERROR called directly");
          }
        | RESET
          {
            memcached_reset(context->memc);
          }
        | PARSER_DEBUG
          {
            yydebug= 1;
          }
        | INCLUDE ' ' string
          {
            if ((context->rc= memcached_parse_configure_file(*context->memc, $3.c_str, $3.size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Failed to parse configuration file");
            }
          }
        ;


expression:
          SERVER HOSTNAME optional_port optional_weight
          {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, $2.c_str, $3, uint32_t($4))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", $2.c_str, uint32_t($3));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
        | SERVER IPADDRESS optional_port optional_weight
          {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, $2.c_str, $3, uint32_t($4))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", $2.c_str, uint32_t($3));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
        | CSL_SOCKET string optional_weight
          {
            if (memcached_failed(context->rc= memcached_server_add_unix_socket_with_weight(context->memc, $2.c_str, uint32_t($3))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add socket: %s", $2.c_str);
              parser_abort(context, buffer);
            }
          }
        | CONFIGURE_FILE string
          {
            memcached_set_configuration_file(context->memc, $2.c_str, $2.size);
          }
        | POOL_MIN NUMBER
          {
            context->memc->configure.initial_pool_size= uint32_t($2);
          }
        | POOL_MAX NUMBER
          {
            context->memc->configure.max_pool_size= uint32_t($2);
          }
        | behaviors
        ;

behaviors:
          NAMESPACE string
          {
            if (memcached_callback_get(context->memc, MEMCACHED_CALLBACK_PREFIX_KEY, NULL))
            {
              parser_abort(context, "--NAMESPACE can only be called once");
            }

            if ((context->rc= memcached_set_namespace(*context->memc, $2.c_str, $2.size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));
            }
          }
        | FETCH_VERSION
          {
            memcached_flag(*context->memc, MEMCACHED_FLAG_IS_FETCHING_VERSION, true);
          }
        | DISTRIBUTION distribution
          {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, $2)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, $2)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));;
            }
          }
        | DISTRIBUTION distribution ',' hash
          {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, $2)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set_distribution_hash(context->memc, $4)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set the hash for the DISTRIBUTION requested");
            }
          }
        | HASH hash
          {
            if (context->set_hash($2) == false)
            {
              parser_abort(context, "--HASH can only be set once");
            }
          }
        | behavior_number NUMBER
          {
            if ((context->rc= memcached_behavior_set(context->memc, $1, $2)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set behavior");
            }
          }
        | behavior_boolean
          {
            if ((context->rc= memcached_behavior_set(context->memc, $1, true)) != MEMCACHED_SUCCESS)
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Could not set: %s", libmemcached_string_behavior($1));
              parser_abort(context, buffer);
            }
          }
        |  USER_DATA
          {
          }
        ;

behavior_number:
          REMOVE_FAILED_SERVERS
          {
            $$= MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS;
          }
        | CONNECT_TIMEOUT
          {
            $$= MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT;
          }
        | IO_MSG_WATERMARK
          {
            $$= MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK;
          }
        | IO_BYTES_WATERMARK
          {
            $$= MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK;
          }
        | IO_KEY_PREFETCH
          {
            $$= MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH;
          }
        | NUMBER_OF_REPLICAS
          {
            $$= MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS;
          }
        | POLL_TIMEOUT
          {
            $$= MEMCACHED_BEHAVIOR_POLL_TIMEOUT;
          }
        |  RCV_TIMEOUT
          {
            $$= MEMCACHED_BEHAVIOR_RCV_TIMEOUT;
          }
        |  RETRY_TIMEOUT
          {
            $$= MEMCACHED_BEHAVIOR_RETRY_TIMEOUT;
          }
        |  SND_TIMEOUT
          {
            $$= MEMCACHED_BEHAVIOR_SND_TIMEOUT;
          }
        |  SOCKET_RECV_SIZE
          {
            $$= MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE;
          }
        |  SOCKET_SEND_SIZE
          {
            $$= MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE;
          }
        ;

behavior_boolean: 
          BINARY_PROTOCOL
          {
            $$= MEMCACHED_BEHAVIOR_BINARY_PROTOCOL;
          }
        | BUFFER_REQUESTS
          {
            $$= MEMCACHED_BEHAVIOR_BUFFER_REQUESTS;
          }
        | HASH_WITH_NAMESPACE
          {
            $$= MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY;
          }
        | NOREPLY
          {
            $$= MEMCACHED_BEHAVIOR_NOREPLY;
          }
        |  RANDOMIZE_REPLICA_READ
          {
            $$= MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ;
          }
        |  SORT_HOSTS
          {
            $$= MEMCACHED_BEHAVIOR_SORT_HOSTS;
          }
        |  SUPPORT_CAS
          {
            $$= MEMCACHED_BEHAVIOR_SUPPORT_CAS;
          }
        |  _TCP_NODELAY
          {
            $$= MEMCACHED_BEHAVIOR_TCP_NODELAY;
          }
        |  _TCP_KEEPALIVE
          {
            $$= MEMCACHED_BEHAVIOR_TCP_KEEPALIVE;
          }
        |  _TCP_KEEPIDLE
          {
            $$= MEMCACHED_BEHAVIOR_TCP_KEEPIDLE;
          }
        |  USE_UDP
          {
            $$= MEMCACHED_BEHAVIOR_USE_UDP;
          }
        |  VERIFY_KEY
          {
            $$= MEMCACHED_BEHAVIOR_VERIFY_KEY;
          }


optional_port:
          { $$= MEMCACHED_DEFAULT_PORT;}
        | PORT
          { };
        ;

optional_weight:
          { $$= 1; }
        | WEIGHT_START
          { }
        ;

hash:
          MD5
          {
            $$= MEMCACHED_HASH_MD5;
          }
        | CRC
          {
            $$= MEMCACHED_HASH_CRC;
          }
        | FNV1_64
          {
            $$= MEMCACHED_HASH_FNV1_64;
          }
        | FNV1A_64
          {
            $$= MEMCACHED_HASH_FNV1A_64;
          }
        | FNV1_32
          {
            $$= MEMCACHED_HASH_FNV1_32;
          }
        | FNV1A_32
          {
            $$= MEMCACHED_HASH_FNV1A_32;
          }
        | HSIEH
          {
            $$= MEMCACHED_HASH_HSIEH;
          }
        | MURMUR
          {
            $$= MEMCACHED_HASH_MURMUR;
          }
        | JENKINS
          {
            $$= MEMCACHED_HASH_JENKINS;
          }
        ;

string:
          STRING
          {
            $$= $1;
          }
        | QUOTED_STRING
          {
            $$= $1;
          }
        ;

distribution:
          CONSISTENT
          {
            $$= MEMCACHED_DISTRIBUTION_CONSISTENT;
          }
        | MODULA
          {
            $$= MEMCACHED_DISTRIBUTION_MODULA;
          }
        | RANDOM
          {
            $$= MEMCACHED_DISTRIBUTION_RANDOM;
          }
        ;

%% 

void Context::start() 
{
  config_parse(this, (void **)scanner);
}

