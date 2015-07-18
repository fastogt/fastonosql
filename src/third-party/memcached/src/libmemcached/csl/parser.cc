/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         config_parse
#define yylex           config_lex
#define yyerror         config_error
#define yylval          config_lval
#define yychar          config_char
#define yydebug         config_debug
#define yynerrs         config_nerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 37 "libmemcached/csl/parser.yy"


#include <libmemcached/csl/common.h>

class Context;

/* Line 371 of yacc.c  */
#line 60 "libmemcached/csl/parser.yy"


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



/* Line 371 of yacc.c  */
#line 128 "libmemcached/csl/parser.cc"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
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

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 264 "libmemcached/csl/parser.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   75

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  68
/* YYNRULES -- Number of states.  */
#define YYNSTATES  86

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   327

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    75,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    64,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    65,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    66,
      67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     9,    11,    13,    15,    17,    19,
      21,    23,    27,    32,    37,    41,    44,    47,    50,    52,
      55,    57,    60,    65,    68,    71,    73,    75,    77,    79,
      81,    83,    85,    87,    89,    91,    93,    95,    97,    99,
     101,   103,   105,   107,   109,   111,   113,   115,   117,   119,
     121,   123,   124,   126,   127,   129,   131,   133,   135,   137,
     139,   141,   143,   145,   147,   149,   151,   153,   155
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      77,     0,    -1,    78,    -1,    77,    75,    78,    -1,    79,
      -1,     3,    -1,    10,    -1,     4,    -1,     5,    -1,     6,
      -1,     7,    -1,     8,    75,    86,    -1,    11,    71,    83,
      84,    -1,    11,    70,    83,    84,    -1,    12,    86,    84,
      -1,     9,    86,    -1,    48,    67,    -1,    49,    67,    -1,
      80,    -1,    47,    86,    -1,    46,    -1,    20,    87,    -1,
      20,    87,    64,    85,    -1,    21,    85,    -1,    81,    67,
      -1,    82,    -1,    40,    -1,    33,    -1,    19,    -1,    25,
      -1,    23,    -1,    24,    -1,    29,    -1,    30,    -1,    32,
      -1,    34,    -1,    35,    -1,    36,    -1,    37,    -1,    17,
      -1,    18,    -1,    22,    -1,    28,    -1,    31,    -1,    38,
      -1,    39,    -1,    45,    -1,    43,    -1,    44,    -1,    41,
      -1,    42,    -1,    -1,    68,    -1,    -1,    69,    -1,    50,
      -1,    51,    -1,    52,    -1,    53,    -1,    54,    -1,    55,
      -1,    56,    -1,    57,    -1,    58,    -1,    72,    -1,    73,
      -1,    59,    -1,    60,    -1,    61,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   203,   203,   204,   208,   210,   212,   214,   219,   224,
     228,   232,   243,   253,   263,   272,   276,   280,   284,   288,
     300,   304,   317,   330,   337,   344,   353,   359,   363,   367,
     371,   375,   379,   383,   387,   391,   395,   399,   403,   410,
     414,   418,   422,   426,   430,   434,   438,   442,   446,   450,
     454,   461,   462,   467,   468,   473,   477,   481,   485,   489,
     493,   497,   501,   505,   512,   516,   523,   527,   531
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "COMMENT", "END", "CSL_ERROR", "RESET",
  "PARSER_DEBUG", "INCLUDE", "CONFIGURE_FILE", "EMPTY_LINE", "SERVER",
  "CSL_SOCKET", "SERVERS", "SERVERS_OPTION", "UNKNOWN_OPTION", "UNKNOWN",
  "BINARY_PROTOCOL", "BUFFER_REQUESTS", "CONNECT_TIMEOUT", "DISTRIBUTION",
  "HASH", "HASH_WITH_NAMESPACE", "IO_BYTES_WATERMARK", "IO_KEY_PREFETCH",
  "IO_MSG_WATERMARK", "KETAMA_HASH", "KETAMA_WEIGHTED", "NOREPLY",
  "NUMBER_OF_REPLICAS", "POLL_TIMEOUT", "RANDOMIZE_REPLICA_READ",
  "RCV_TIMEOUT", "REMOVE_FAILED_SERVERS", "RETRY_TIMEOUT", "SND_TIMEOUT",
  "SOCKET_RECV_SIZE", "SOCKET_SEND_SIZE", "SORT_HOSTS", "SUPPORT_CAS",
  "USER_DATA", "USE_UDP", "VERIFY_KEY", "_TCP_KEEPALIVE", "_TCP_KEEPIDLE",
  "_TCP_NODELAY", "FETCH_VERSION", "NAMESPACE", "POOL_MIN", "POOL_MAX",
  "MD5", "CRC", "FNV1_64", "FNV1A_64", "FNV1_32", "FNV1A_32", "HSIEH",
  "MURMUR", "JENKINS", "CONSISTENT", "MODULA", "RANDOM", "CSL_TRUE",
  "CSL_FALSE", "','", "'='", "CSL_FLOAT", "NUMBER", "PORT", "WEIGHT_START",
  "IPADDRESS", "HOSTNAME", "STRING", "QUOTED_STRING", "FILE_PATH", "' '",
  "$accept", "begin", "statement", "expression", "behaviors",
  "behavior_number", "behavior_boolean", "optional_port",
  "optional_weight", "hash", "string", "distribution", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,    44,    61,   319,   320,   321,   322,
     323,   324,   325,   326,   327,    32
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    77,    78,    78,    78,    78,    78,    78,
      78,    78,    79,    79,    79,    79,    79,    79,    79,    80,
      80,    80,    80,    80,    80,    80,    80,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    83,    83,    84,    84,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    86,    86,    87,    87,    87
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     4,     4,     3,     2,     2,     2,     1,     2,
       1,     2,     4,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     5,     7,     8,     9,    10,     0,     0,     6,     0,
       0,    39,    40,    28,     0,     0,    41,    30,    31,    29,
      42,    32,    33,    43,    34,    27,    35,    36,    37,    38,
      44,    45,    26,    49,    50,    47,    48,    46,    20,     0,
       0,     0,     0,     2,     4,    18,     0,    25,     0,    64,
      65,    15,    51,    51,    53,    66,    67,    68,    21,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    23,    19,
      16,    17,     1,     0,    24,    11,    52,    53,    53,    54,
      14,     0,     3,    13,    12,    22
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    42,    43,    44,    45,    46,    47,    77,    80,    68,
      51,    58
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -62
static const yytype_int8 yypact[] =
{
      -2,   -62,   -62,   -62,   -62,   -62,   -61,   -24,   -62,   -20,
     -24,   -62,   -62,   -62,   -48,    13,   -62,   -62,   -62,   -62,
     -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,
     -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -24,
     -42,   -15,     0,   -62,   -62,   -62,   -11,   -62,   -24,   -62,
     -62,   -62,   -10,   -10,   -12,   -62,   -62,   -62,    -5,   -62,
     -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,   -62,
     -62,   -62,   -62,    -2,   -62,   -62,   -62,   -12,   -12,   -62,
     -62,    13,   -62,   -62,   -62,   -62
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -62,   -62,   -13,   -62,   -62,   -62,   -62,     8,   -23,    -9,
      14,   -62
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      72,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    55,    56,    57,    48,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    54,    70,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    49,    50,
      52,    53,    71,    69,    83,    84,    74,    79,    76,    81,
      82,    78,    75,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    85,     0,     0,    73
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-62)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int8 yycheck[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    59,    60,    61,    75,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    10,    67,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    72,    73,
      70,    71,    67,    39,    77,    78,    67,    69,    68,    64,
      73,    53,    48,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    81,    -1,    -1,    75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    77,    78,    79,    80,    81,    82,    75,    72,
      73,    86,    70,    71,    86,    59,    60,    61,    87,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    85,    86,
      67,    67,     0,    75,    67,    86,    68,    83,    83,    69,
      84,    64,    78,    84,    84,    85
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (context, scanner, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, scanner)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, context, scanner); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, class Context *context, yyscan_t *scanner)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, context, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    class Context *context;
    yyscan_t *scanner;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (context);
  YYUSE (scanner);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, class Context *context, yyscan_t *scanner)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, context, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    class Context *context;
    yyscan_t *scanner;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, context, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, class Context *context, yyscan_t *scanner)
#else
static void
yy_reduce_print (yyvsp, yyrule, context, scanner)
    YYSTYPE *yyvsp;
    int yyrule;
    class Context *context;
    yyscan_t *scanner;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , context, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, context, scanner); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, class Context *context, yyscan_t *scanner)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, context, scanner)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    class Context *context;
    yyscan_t *scanner;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (context);
  YYUSE (scanner);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (class Context *context, yyscan_t *scanner)
#else
int
yyparse (context, scanner)
    class Context *context;
    yyscan_t *scanner;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
/* Line 1792 of yacc.c  */
#line 209 "libmemcached/csl/parser.yy"
    { }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 211 "libmemcached/csl/parser.yy"
    { }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 213 "libmemcached/csl/parser.yy"
    { }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 215 "libmemcached/csl/parser.yy"
    {
            context->set_end();
            YYACCEPT;
          }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 220 "libmemcached/csl/parser.yy"
    {
            context->rc= MEMCACHED_PARSE_USER_ERROR;
            parser_abort(context, "ERROR called directly");
          }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 225 "libmemcached/csl/parser.yy"
    {
            memcached_reset(context->memc);
          }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 229 "libmemcached/csl/parser.yy"
    {
            yydebug= 1;
          }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 233 "libmemcached/csl/parser.yy"
    {
            if ((context->rc= memcached_parse_configure_file(*context->memc, (yyvsp[(3) - (3)].string).c_str, (yyvsp[(3) - (3)].string).size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Failed to parse configuration file");
            }
          }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 244 "libmemcached/csl/parser.yy"
    {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, (yyvsp[(2) - (4)].server).c_str, (yyvsp[(3) - (4)].number), uint32_t((yyvsp[(4) - (4)].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", (yyvsp[(2) - (4)].server).c_str, uint32_t((yyvsp[(3) - (4)].number)));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 254 "libmemcached/csl/parser.yy"
    {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, (yyvsp[(2) - (4)].server).c_str, (yyvsp[(3) - (4)].number), uint32_t((yyvsp[(4) - (4)].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", (yyvsp[(2) - (4)].server).c_str, uint32_t((yyvsp[(3) - (4)].number)));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 264 "libmemcached/csl/parser.yy"
    {
            if (memcached_failed(context->rc= memcached_server_add_unix_socket_with_weight(context->memc, (yyvsp[(2) - (3)].string).c_str, uint32_t((yyvsp[(3) - (3)].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add socket: %s", (yyvsp[(2) - (3)].string).c_str);
              parser_abort(context, buffer);
            }
          }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 273 "libmemcached/csl/parser.yy"
    {
            memcached_set_configuration_file(context->memc, (yyvsp[(2) - (2)].string).c_str, (yyvsp[(2) - (2)].string).size);
          }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 277 "libmemcached/csl/parser.yy"
    {
            context->memc->configure.initial_pool_size= uint32_t((yyvsp[(2) - (2)].number));
          }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 281 "libmemcached/csl/parser.yy"
    {
            context->memc->configure.max_pool_size= uint32_t((yyvsp[(2) - (2)].number));
          }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 289 "libmemcached/csl/parser.yy"
    {
            if (memcached_callback_get(context->memc, MEMCACHED_CALLBACK_PREFIX_KEY, NULL))
            {
              parser_abort(context, "--NAMESPACE can only be called once");
            }

            if ((context->rc= memcached_set_namespace(*context->memc, (yyvsp[(2) - (2)].string).c_str, (yyvsp[(2) - (2)].string).size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));
            }
          }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 301 "libmemcached/csl/parser.yy"
    {
            memcached_flag(*context->memc, MEMCACHED_FLAG_IS_FETCHING_VERSION, true);
          }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 305 "libmemcached/csl/parser.yy"
    {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[(2) - (2)].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[(2) - (2)].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));;
            }
          }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 318 "libmemcached/csl/parser.yy"
    {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[(2) - (4)].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set_distribution_hash(context->memc, (yyvsp[(4) - (4)].hash))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set the hash for the DISTRIBUTION requested");
            }
          }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 331 "libmemcached/csl/parser.yy"
    {
            if (context->set_hash((yyvsp[(2) - (2)].hash)) == false)
            {
              parser_abort(context, "--HASH can only be set once");
            }
          }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 338 "libmemcached/csl/parser.yy"
    {
            if ((context->rc= memcached_behavior_set(context->memc, (yyvsp[(1) - (2)].behavior), (yyvsp[(2) - (2)].number))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set behavior");
            }
          }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 345 "libmemcached/csl/parser.yy"
    {
            if ((context->rc= memcached_behavior_set(context->memc, (yyvsp[(1) - (1)].behavior), true)) != MEMCACHED_SUCCESS)
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Could not set: %s", libmemcached_string_behavior((yyvsp[(1) - (1)].behavior)));
              parser_abort(context, buffer);
            }
          }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 354 "libmemcached/csl/parser.yy"
    {
          }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 360 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS;
          }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 364 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT;
          }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 368 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK;
          }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 372 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK;
          }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 376 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH;
          }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 380 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS;
          }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 384 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_POLL_TIMEOUT;
          }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 388 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RCV_TIMEOUT;
          }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 392 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RETRY_TIMEOUT;
          }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 396 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SND_TIMEOUT;
          }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 400 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE;
          }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 404 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE;
          }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 411 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_BINARY_PROTOCOL;
          }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 415 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_BUFFER_REQUESTS;
          }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 419 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY;
          }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 423 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NOREPLY;
          }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 427 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ;
          }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 431 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SORT_HOSTS;
          }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 435 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SUPPORT_CAS;
          }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 439 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_NODELAY;
          }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 443 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_KEEPALIVE;
          }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 447 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_KEEPIDLE;
          }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 451 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_USE_UDP;
          }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 455 "libmemcached/csl/parser.yy"
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_VERIFY_KEY;
          }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 461 "libmemcached/csl/parser.yy"
    { (yyval.number)= MEMCACHED_DEFAULT_PORT;}
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 463 "libmemcached/csl/parser.yy"
    { }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 467 "libmemcached/csl/parser.yy"
    { (yyval.number)= 1; }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 469 "libmemcached/csl/parser.yy"
    { }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 474 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_MD5;
          }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 478 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_CRC;
          }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 482 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1_64;
          }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 486 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1A_64;
          }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 490 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1_32;
          }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 494 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1A_32;
          }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 498 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_HSIEH;
          }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 502 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_MURMUR;
          }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 506 "libmemcached/csl/parser.yy"
    {
            (yyval.hash)= MEMCACHED_HASH_JENKINS;
          }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 513 "libmemcached/csl/parser.yy"
    {
            (yyval.string)= (yyvsp[(1) - (1)].string);
          }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 517 "libmemcached/csl/parser.yy"
    {
            (yyval.string)= (yyvsp[(1) - (1)].string);
          }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 524 "libmemcached/csl/parser.yy"
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_CONSISTENT;
          }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 528 "libmemcached/csl/parser.yy"
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_MODULA;
          }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 532 "libmemcached/csl/parser.yy"
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_RANDOM;
          }
    break;


/* Line 1792 of yacc.c  */
#line 2130 "libmemcached/csl/parser.cc"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (context, scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (context, scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, context, scanner);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, context, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (context, scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, context, scanner);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, context, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 537 "libmemcached/csl/parser.yy"
 

void Context::start() 
{
  config_parse(this, (void **)scanner);
}

