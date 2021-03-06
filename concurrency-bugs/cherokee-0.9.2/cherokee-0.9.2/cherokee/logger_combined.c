/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Cherokee
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2001-2008 Alvaro Lopez Ortega
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "logger_combined.h"
#include "logger_ncsa.h"
#include "plugin_loader.h"


/* Plug-in definition
 */
PLUGIN_INFO_LOGGER_EASIEST_INIT(combined);


ret_t
cherokee_logger_combined_new (cherokee_logger_t **logger, cherokee_config_node_t *config)
{
	ret_t ret;
	CHEROKEE_NEW_STRUCT(n, logger_combined);
	
	/* Init the base class object
	 */
	cherokee_logger_init_base (LOGGER(n), PLUGIN_INFO_PTR(combined));

	MODULE(n)->init         = (logger_func_init_t) cherokee_logger_ncsa_init;
	MODULE(n)->free         = (logger_func_free_t) cherokee_logger_ncsa_free;
	LOGGER(n)->flush        = (logger_func_flush_t) cherokee_logger_ncsa_flush;
	LOGGER(n)->reopen       = (logger_func_flush_t) cherokee_logger_ncsa_reopen;
	LOGGER(n)->write_error  = (logger_func_write_error_t)  cherokee_logger_ncsa_write_error;
	LOGGER(n)->write_access = (logger_func_write_access_t) cherokee_logger_ncsa_write_access;
	LOGGER(n)->write_string = (logger_func_write_string_t) cherokee_logger_ncsa_write_string;

	/* Init the base class: NCSA
	 */
	ret = cherokee_logger_ncsa_init_base (n, config);
	if (unlikely(ret < ret_ok)) return ret;

	/* Active the "Combined" bit
	 */
	n->combined = true;

	/* Return the object
	 */
	*logger = LOGGER(n);
	return ret_ok;
}


/* Library init function
 */
static cherokee_boolean_t _combined_is_init = false;

void                                                        
PLUGIN_INIT_NAME(name) (cherokee_plugin_loader_t *loader)   
{                                                           
	if (_combined_is_init) return;
	_combined_is_init = true;

	cherokee_plugin_loader_load (loader, "ncsa");
}                                                           
