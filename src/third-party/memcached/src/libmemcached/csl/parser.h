/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED
# define YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int config_debug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     COMMENT = 258,
     END = 259,
     CSL_ERROR = 260,
     RESET = 261,
     PARSER_DEBUG = 262,
     INCLUDE = 263,
     CONFIGURE_FILE = 264,
     EMPTY_LINE = 265,
     SERVER = 266,
     CSL_SOCKET = 267,
     SERVERS = 268,
     SERVERS_OPTION = 269,
     UNKNOWN_OPTION = 270,
     UNKNOWN = 271,
     BINARY_PROTOCOL = 272,
     BUFFER_REQUESTS = 273,
     CONNECT_TIMEOUT = 274,
     DISTRIBUTION = 275,
     HASH = 276,
     HASH_WITH_NAMESPACE = 277,
     IO_BYTES_WATERMARK = 278,
     IO_KEY_PREFETCH = 279,
     IO_MSG_WATERMARK = 280,
     KETAMA_HASH = 281,
     KETAMA_WEIGHTED = 282,
     NOREPLY = 283,
     NUMBER_OF_REPLICAS = 284,
     POLL_TIMEOUT = 285,
     RANDOMIZE_REPLICA_READ = 286,
     RCV_TIMEOUT = 287,
     REMOVE_FAILED_SERVERS = 288,
     RETRY_TIMEOUT = 289,
     SND_TIMEOUT = 290,
     SOCKET_RECV_SIZE = 291,
     SOCKET_SEND_SIZE = 292,
     SORT_HOSTS = 293,
     SUPPORT_CAS = 294,
     USER_DATA = 295,
     USE_UDP = 296,
     VERIFY_KEY = 297,
     _TCP_KEEPALIVE = 298,
     _TCP_KEEPIDLE = 299,
     _TCP_NODELAY = 300,
     FETCH_VERSION = 301,
     NAMESPACE = 302,
     POOL_MIN = 303,
     POOL_MAX = 304,
     MD5 = 305,
     CRC = 306,
     FNV1_64 = 307,
     FNV1A_64 = 308,
     FNV1_32 = 309,
     FNV1A_32 = 310,
     HSIEH = 311,
     MURMUR = 312,
     JENKINS = 313,
     CONSISTENT = 314,
     MODULA = 315,
     RANDOM = 316,
     CSL_TRUE = 317,
     CSL_FALSE = 318,
     CSL_FLOAT = 319,
     NUMBER = 320,
     PORT = 321,
     WEIGHT_START = 322,
     IPADDRESS = 323,
     HOSTNAME = 324,
     STRING = 325,
     QUOTED_STRING = 326,
     FILE_PATH = 327
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int config_parse (void *YYPARSE_PARAM);
#else
int config_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int config_parse (class Context *context, yyscan_t *scanner);
#else
int config_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED  */
