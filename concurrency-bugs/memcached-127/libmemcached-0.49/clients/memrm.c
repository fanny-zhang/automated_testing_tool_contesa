/* LibMemcached
 * Copyright (C) 2006-2009 Brian Aker
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 *
 * Summary:
 *
 */
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <libmemcached/memcached.h>
#include <string.h>
#include "client_options.h"
#include "utilities.h"

static int opt_binary= 0;
static int opt_verbose= 0;
static time_t opt_expire= 0;
static char *opt_servers= NULL;
static char *opt_hash= NULL;
static char *opt_username;
static char *opt_passwd;

#define PROGRAM_NAME "memrm"
#define PROGRAM_DESCRIPTION "Erase a key or set of keys from a memcached cluster."

/* Prototypes */
static void options_parse(int argc, char *argv[]);

int main(int argc, char *argv[])
{
  memcached_st *memc;
  memcached_return_t rc;
  memcached_server_st *servers;

  int return_code= 0;

  options_parse(argc, argv);
  initialize_sockets();

  if (!opt_servers)
  {
    char *temp;

    if ((temp= getenv("MEMCACHED_SERVERS")))
      opt_servers= strdup(temp);
    else
    {
      fprintf(stderr, "No Servers provided\n");
      exit(1);
    }
  }

  memc= memcached_create(NULL);
  process_hash_option(memc, opt_hash);

  servers= memcached_servers_parse(opt_servers);
  memcached_server_push(memc, servers);
  memcached_server_list_free(servers);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL,
                         (uint64_t) opt_binary);

  if (!initialize_sasl(memc, opt_username, opt_passwd))
  {
    memcached_free(memc);
    return EXIT_FAILURE;
  }

  while (optind < argc)
  {
    if (opt_verbose)
      printf("key: %s\nexpires: %llu\n", argv[optind], (unsigned long long)opt_expire);
    rc = memcached_delete(memc, argv[optind], strlen(argv[optind]), opt_expire);

    if (rc != MEMCACHED_SUCCESS)
    {
      fprintf(stderr, "memrm: %s: memcache error %s",
	      argv[optind], memcached_strerror(memc, rc));
      if (memcached_last_error_errno(memc))
	fprintf(stderr, " system error %s", strerror(memcached_last_error_errno(memc)));
      fprintf(stderr, "\n");

      return_code= -1;
    }

    optind++;
  }

  memcached_free(memc);

  if (opt_servers)
    free(opt_servers);

  if (opt_hash)
    free(opt_hash);

  shutdown_sasl();

  return return_code;
}


static void options_parse(int argc, char *argv[])
{
  memcached_programs_help_st help_options[]=
  {
    {0},
  };

  static struct option long_options[]=
  {
    {(OPTIONSTRING)"version", no_argument, NULL, OPT_VERSION},
    {(OPTIONSTRING)"help", no_argument, NULL, OPT_HELP},
    {(OPTIONSTRING)"verbose", no_argument, &opt_verbose, OPT_VERBOSE},
    {(OPTIONSTRING)"debug", no_argument, &opt_verbose, OPT_DEBUG},
    {(OPTIONSTRING)"servers", required_argument, NULL, OPT_SERVERS},
    {(OPTIONSTRING)"expire", required_argument, NULL, OPT_EXPIRE},
    {(OPTIONSTRING)"hash", required_argument, NULL, OPT_HASH},
    {(OPTIONSTRING)"binary", no_argument, NULL, OPT_BINARY},
    {(OPTIONSTRING)"username", required_argument, NULL, OPT_USERNAME},
    {(OPTIONSTRING)"password", required_argument, NULL, OPT_PASSWD},
    {0, 0, 0, 0},
  };
  int option_index= 0;
  int option_rv;

  while (1)
  {
    option_rv= getopt_long(argc, argv, "Vhvds:", long_options, &option_index);
    if (option_rv == -1) break;
    switch (option_rv)
    {
    case 0:
      break;
    case OPT_BINARY:
      opt_binary = 1;
      break;
    case OPT_VERBOSE: /* --verbose or -v */
      opt_verbose = OPT_VERBOSE;
      break;
    case OPT_DEBUG: /* --debug or -d */
      opt_verbose = OPT_DEBUG;
      break;
    case OPT_VERSION: /* --version or -V */
      version_command(PROGRAM_NAME);
      break;
    case OPT_HELP: /* --help or -h */
      help_command(PROGRAM_NAME, PROGRAM_DESCRIPTION, long_options, help_options);
      break;
    case OPT_SERVERS: /* --servers or -s */
      opt_servers= strdup(optarg);
      break;
    case OPT_EXPIRE: /* --expire */
      opt_expire= (time_t)strtoll(optarg, (char **)NULL, 10);
      break;
    case OPT_HASH:
      opt_hash= strdup(optarg);
      break;
    case OPT_USERNAME:
      opt_username= optarg;
      break;
    case OPT_PASSWD:
      opt_passwd= optarg;
      break;
    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
    default:
      abort();
    }
  }
}
