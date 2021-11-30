/*
 * modules.c --- automatically generated by Apache
 * configuration script.  DO NOT HAND EDIT!!!!!
 */

#include "ap_config.h"
#include "httpd.h"
#include "http_config.h"

extern module core_module;
extern module access_module;
extern module auth_module;
extern module include_module;
extern module log_config_module;
extern module env_module;
extern module setenvif_module;
extern module mpm_prefork_module;
extern module http_module;
extern module mime_module;
extern module status_module;
extern module autoindex_module;
extern module asis_module;
extern module cgi_module;
extern module negotiation_module;
extern module dir_module;
extern module imap_module;
extern module actions_module;
extern module userdir_module;
extern module alias_module;
extern module so_module;

/*
 *  Modules which implicitly form the
 *  list of activated modules on startup,
 *  i.e. these are the modules which are
 *  initially linked into the Apache processing
 *  [extendable under run-time via AddModule]
 */
module *ap_prelinked_modules[] = {
  &core_module,
  &access_module,
  &auth_module,
  &include_module,
  &log_config_module,
  &env_module,
  &setenvif_module,
  &mpm_prefork_module,
  &http_module,
  &mime_module,
  &status_module,
  &autoindex_module,
  &asis_module,
  &cgi_module,
  &negotiation_module,
  &dir_module,
  &imap_module,
  &actions_module,
  &userdir_module,
  &alias_module,
  &so_module,
  NULL
};

/*
 *  Modules which initially form the
 *  list of available modules on startup,
 *  i.e. these are the modules which are
 *  initially loaded into the Apache process
 *  [extendable under run-time via LoadModule]
 */
module *ap_preloaded_modules[] = {
  &core_module,
  &access_module,
  &auth_module,
  &include_module,
  &log_config_module,
  &env_module,
  &setenvif_module,
  &mpm_prefork_module,
  &http_module,
  &mime_module,
  &status_module,
  &autoindex_module,
  &asis_module,
  &cgi_module,
  &negotiation_module,
  &dir_module,
  &imap_module,
  &actions_module,
  &userdir_module,
  &alias_module,
  &so_module,
  NULL
};

