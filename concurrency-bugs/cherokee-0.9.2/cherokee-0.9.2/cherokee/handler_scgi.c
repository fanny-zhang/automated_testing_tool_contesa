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

#include "common-internal.h"
#include "handler_scgi.h"

#include "connection.h"
#include "source_interpreter.h"
#include "thread.h"
#include "util.h"
#include "connection-protected.h"
#include "bogotime.h"

#define ENTRIES "handler,cgi"

#define set_env(cgi,key,val,len) \
	add_env_pair (cgi, key, sizeof(key)-1, val, len)

/* Plug-in initialization
 */
CGI_LIB_INIT (scgi, http_get | http_post | http_head);

/* Methods implementation
 */
static ret_t 
props_free (cherokee_handler_scgi_props_t *props)
{
	if (props->balancer)
		cherokee_balancer_free (props->balancer);

	/* TODO: Free scgi_env_ref
	 */

	return cherokee_handler_cgi_base_props_free (PROP_CGI_BASE(props));
}

ret_t 
cherokee_handler_scgi_configure (cherokee_config_node_t *conf, cherokee_server_t *srv, cherokee_module_props_t **_props)
{
	ret_t                          ret;
	cherokee_list_t               *i;
	cherokee_handler_scgi_props_t *props;
	
	/* Instance a new property object
	 */
	if (*_props == NULL) {
		CHEROKEE_NEW_STRUCT (n, handler_scgi_props);

		cherokee_handler_cgi_base_props_init_base (PROP_CGI_BASE(n), 
							   MODULE_PROPS_FREE(props_free));

		INIT_LIST_HEAD(&n->scgi_env_ref);   // TODO: finish this
		n->balancer = NULL;

		*_props = MODULE_PROPS(n);
	}

	props = PROP_SCGI(*_props);	

	/* Parse the configuration tree
	 */
	cherokee_config_node_foreach (i, conf) {
		cherokee_config_node_t *subconf = CONFIG_NODE(i);

		if (equal_buf_str (&subconf->key, "balancer")) {
			ret = cherokee_balancer_instance (&subconf->val, subconf, srv, &props->balancer); 
			if (ret != ret_ok) return ret;
		}
	}

	/* Init base class
	 */
	ret = cherokee_handler_cgi_base_configure (conf, srv, _props);
	if (ret != ret_ok) return ret;

	/* Final checks
	 */
	if (props->balancer == NULL) {
		PRINT_ERROR_S ("ERROR: SCGI handler needs a balancer\n");
		return ret_error;
	}

	return ret_ok;
}


static void 
add_env_pair (cherokee_handler_cgi_base_t *cgi_base, 
	      char *key, int key_len, 
	      char *val, int val_len)
{
	static char              zero = '\0';
	cherokee_handler_scgi_t *scgi = HDL_SCGI(cgi_base);

	cherokee_buffer_ensure_size (&scgi->header, scgi->header.len + key_len + val_len + 3);

	cherokee_buffer_add (&scgi->header, key, key_len);
	cherokee_buffer_add (&scgi->header, &zero, 1);
	cherokee_buffer_add (&scgi->header, val, val_len);
	cherokee_buffer_add (&scgi->header, &zero, 1);
}


static ret_t
read_from_scgi (cherokee_handler_cgi_base_t *cgi_base, cherokee_buffer_t *buffer)
{
	ret_t                    ret;
	size_t                   read = 0;
	cherokee_handler_scgi_t *scgi = HDL_SCGI(cgi_base);
	
	ret = cherokee_socket_bufread (&scgi->socket, buffer, 4096, &read);

	switch (ret) {
	case ret_eagain:
		cherokee_thread_deactive_to_polling (HANDLER_THREAD(cgi_base), HANDLER_CONN(cgi_base), 
						     scgi->socket.socket, 0, false);
		return ret_eagain;

	case ret_ok:
		TRACE (ENTRIES, "%d bytes read\n", read);
		return ret_ok;

	case ret_eof:
	case ret_error:
		cgi_base->got_eof = true;
		return ret;

	default:
		RET_UNKNOWN(ret);
	}

	SHOULDNT_HAPPEN;
	return ret_error;	
}


ret_t 
cherokee_handler_scgi_new (cherokee_handler_t **hdl, void *cnt, cherokee_module_props_t *props)
{
	CHEROKEE_NEW_STRUCT (n, handler_scgi);
	
	/* Init the base class
	 */
	cherokee_handler_cgi_base_init (
			HDL_CGI_BASE(n), cnt,
			PLUGIN_INFO_HANDLER_PTR(scgi), 
			HANDLER_PROPS(props),
			add_env_pair, read_from_scgi);

	/* Virtual methods
	 */
	MODULE(n)->init         = (handler_func_init_t) cherokee_handler_scgi_init;
	MODULE(n)->free         = (module_func_free_t) cherokee_handler_scgi_free;

	/* Virtual methods: implemented by handler_cgi_base
	 */
	HANDLER(n)->step        = (handler_func_step_t) cherokee_handler_cgi_base_step;
	HANDLER(n)->add_headers = (handler_func_add_headers_t) cherokee_handler_cgi_base_add_headers;

	/* Properties
	 */
	n->post_len = 0;
	n->spawned  = 0;
	n->src_ref  = NULL;

	cherokee_buffer_init (&n->header);
	cherokee_socket_init (&n->socket);

	/* Return the object
	 */
	*hdl = HANDLER(n);
	return ret_ok;	   
}


ret_t 
cherokee_handler_scgi_free (cherokee_handler_scgi_t *hdl)
{
	/* Free the rest of the handler CGI memory
	 */
	cherokee_handler_cgi_base_free (HDL_CGI_BASE(hdl));

	/* SCGI stuff
	 */
	cherokee_socket_close (&hdl->socket);
	cherokee_socket_mrproper (&hdl->socket);

	cherokee_buffer_mrproper (&hdl->header);

	return ret_ok;
}


static ret_t
netstringer (cherokee_buffer_t *buf)
{
	cint_t len;
	CHEROKEE_TEMP(num,16);

	len = snprintf (num, num_size, "%d:", buf->len);
	if (len < 0)
		return ret_error;

	cherokee_buffer_ensure_size (buf, buf->len + len + 2);
	cherokee_buffer_prepend (buf, num, len);
	cherokee_buffer_add (buf, ",", 1);

#if 0
	cherokee_buffer_print_debug (buf, -1);
#endif	

	return ret_ok;
}


static ret_t
build_header (cherokee_handler_scgi_t *hdl)
{
	cuint_t len;
	char    tmp[64];

	len = snprintf (tmp, sizeof(tmp), FMT_OFFSET, (CST_OFFSET)hdl->post_len);
	
	set_env (HDL_CGI_BASE(hdl), "CONTENT_LENGTH", tmp, len);
	set_env (HDL_CGI_BASE(hdl), "SCGI", "1", 1);

	cherokee_handler_cgi_base_build_envp (HDL_CGI_BASE(hdl), HANDLER_CONN(hdl));       	

	return netstringer (&hdl->header);
}



static ret_t 
connect_to_server (cherokee_handler_scgi_t *hdl)
{
	ret_t                          ret;
	cherokee_connection_t         *conn  = HANDLER_CONN(hdl);
	cherokee_handler_scgi_props_t *props = HANDLER_SCGI_PROPS(hdl);

	/* Get a reference to the target host
	 */
	if (hdl->src_ref == NULL) {
		ret = cherokee_balancer_dispatch (props->balancer, conn, &hdl->src_ref);
		if (ret != ret_ok)
			return ret;
	}

	/* Try to connect
	 */
	if (hdl->src_ref->type == source_host) {
		ret = cherokee_source_connect_polling (hdl->src_ref, 
						       &hdl->socket, conn);		
	} else {
		ret = cherokee_source_interpreter_connect_polling (SOURCE_INT(hdl->src_ref),
								   &hdl->socket, conn, 
								   &hdl->spawned);
	}

	return ret;
}


static ret_t
send_header (cherokee_handler_scgi_t *hdl)
{
	ret_t                  ret;
	size_t                 written = 0;
	cherokee_connection_t *conn    = HANDLER_CONN(hdl);
	
	ret = cherokee_socket_bufwrite (&hdl->socket, &hdl->header, &written);
	if (ret != ret_ok) {
		conn->error_code = http_bad_gateway;
		return ret;
	}

#if 0	
	cherokee_buffer_print_debug (&hdl->header, -1);
#endif
	cherokee_buffer_move_to_begin (&hdl->header, written);

	TRACE (ENTRIES, "sent remaining=%d\n", hdl->header.len);

	if (! cherokee_buffer_is_empty (&hdl->header))
		return ret_eagain;
	
	return ret_ok;
}


static ret_t
send_post (cherokee_handler_scgi_t *hdl)
{
	ret_t                  ret;
	int                    e_fd = -1;
	int                    mode =  0;
	cherokee_connection_t *conn = HANDLER_CONN(hdl);
	
	ret = cherokee_post_walk_to_fd (&conn->post, hdl->socket.socket, &e_fd, &mode);
	
	switch (ret) {
	case ret_ok:
		break;
	case ret_eagain:
		if (e_fd != -1)
			cherokee_thread_deactive_to_polling (HANDLER_THREAD(hdl), conn, e_fd, mode, false);
		return ret_eagain;
	default:
		conn->error_code = http_bad_gateway;
		return ret;
	}

	return ret_ok;
}


ret_t 
cherokee_handler_scgi_init (cherokee_handler_scgi_t *hdl)
{
	ret_t                  ret;
	cherokee_connection_t *conn = HANDLER_CONN(hdl);

	switch (HDL_CGI_BASE(hdl)->init_phase) {
	case hcgi_phase_build_headers:
		TRACE (ENTRIES, "Init: %s\n", "begins");

		/* Extracts PATH_INFO and filename from request uri 
		 */
		ret = cherokee_handler_cgi_base_extract_path (HDL_CGI_BASE(hdl), false);
		if (unlikely (ret < ret_ok)) {
			conn->error_code = http_internal_error;
			return ret_error;
		}
		
		/* Prepare Post
		 */
		if (! cherokee_post_is_empty (&conn->post)) {
			cherokee_post_walk_reset (&conn->post);
			cherokee_post_get_len (&conn->post, &hdl->post_len);
		}

		/* Build the headers
		 */
		ret = build_header (hdl);
		if (unlikely (ret != ret_ok)) {
			conn->error_code = http_internal_error;
			return ret_error;
		}

		HDL_CGI_BASE(hdl)->init_phase = hcgi_phase_connect;

	case hcgi_phase_connect:
		TRACE (ENTRIES, "Init: %s\n", "connect");

		/* Connect	
		 */
		ret = connect_to_server (hdl);
		switch (ret) {
		case ret_ok:
			break;
		case ret_eagain:
			return ret_eagain;
		case ret_deny:
			conn->error_code = http_gateway_timeout;
			return ret_error;
		default:
			conn->error_code = http_service_unavailable;
			return ret_error;
		}
		
		HDL_CGI_BASE(hdl)->init_phase = hcgi_phase_send_headers;

	case hcgi_phase_send_headers:
		TRACE (ENTRIES, "Init: %s\n", "send_headers");

		/* Send the header
		 */
		ret = send_header (hdl);
		if (ret != ret_ok) 
			return ret;

		HDL_CGI_BASE(hdl)->init_phase = hcgi_phase_send_post;

	case hcgi_phase_send_post:
		/* Send the Post
		 */
		if (hdl->post_len > 0) {
			return send_post (hdl);
		}
		break;
	}

	return ret_ok;
}
