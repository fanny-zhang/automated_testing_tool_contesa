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
#include "buffer.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "md5.h"
#include "util.h"
#include "crc32.h"
#include "sha1.h"

#define ENTRIES "core,buffer"

#define REALLOC_EXTRA_SIZE     16
#define IOS_NUMBUF             64	/* I/O size of digits buffer */

#define TO_HEX(c)               ((c) > 9 ? (c) + 'a' - 10 : (c) + '0')


/* Implements _new() and _free() 
 */
CHEROKEE_ADD_FUNC_NEW  (buffer);
CHEROKEE_ADD_FUNC_FREE (buffer);

ret_t 
cherokee_buffer_init (cherokee_buffer_t *buf)
{
	buf->buf  = NULL;
	buf->len  = 0;
	buf->size = 0;

	return ret_ok;
}

void
cherokee_buffer_fake (cherokee_buffer_t *buf, const char *str, cuint_t len)
{
	buf->buf  = (char *)str;
	buf->len  = len;
	buf->size = len + 1;
}


ret_t 
cherokee_buffer_mrproper (cherokee_buffer_t *buf)
{
	if (buf->buf) {
		free (buf->buf);
		buf->buf = NULL;
	}

	buf->len  = 0;
	buf->size = 0;

	return ret_ok;
}

void
cherokee_buffer_clean (cherokee_buffer_t *buf)
{
	if (buf->buf != NULL)
		buf->buf[0] = '\0';
	buf->len = 0;
}

void
cherokee_buffer_swap_buffers (cherokee_buffer_t *buf, cherokee_buffer_t *second)
{
	char    *tmp_buf;
	cuint_t  tmp_len;
	cuint_t  tmp_size;

	tmp_buf  = buf->buf;
	tmp_len  = buf->len;
	tmp_size = buf->size;

	buf->buf  = second->buf;
	buf->len  = second->len;
	buf->size = second->size;

	second->buf = tmp_buf;
	second->len = tmp_len;
	second->size = tmp_size;
}

ret_t 
cherokee_buffer_dup (cherokee_buffer_t *buf, cherokee_buffer_t **dup)
{
	CHEROKEE_NEW_STRUCT(n, buffer);

	n->buf = (char *) malloc(buf->len + 1);
	if (unlikely (n->buf == NULL)) {
		free(n);
		return ret_nomem;
	}

	memcpy (n->buf, buf->buf, buf->len + 1);

	n->len  = buf->len;
	n->size = buf->len + 1;

	*dup = n;
	return ret_ok;
}


static ret_t
realloc_inc_bufsize (cherokee_buffer_t *buf, size_t incsize)
{
	char *pbuf;
	size_t newsize = buf->size + incsize + REALLOC_EXTRA_SIZE + 1;

	pbuf = (char *) realloc(buf->buf, newsize);
	if (unlikely (pbuf == NULL)) 
		return ret_nomem;
	buf->buf = pbuf;
	buf->size = (int) newsize;

	return ret_ok;
}


static ret_t
realloc_new_bufsize (cherokee_buffer_t *buf, size_t newsize)
{
	char *pbuf;
	newsize += REALLOC_EXTRA_SIZE + 1;

	pbuf = (char *) realloc(buf->buf, newsize);
	if (unlikely (pbuf == NULL)) 
		return ret_nomem;
	buf->buf = pbuf;
	buf->size = (int) newsize;

	return ret_ok;
}


ret_t
cherokee_buffer_add (cherokee_buffer_t *buf, const char *txt, size_t size)
{	   
	int free = buf->size - buf->len;

	if (unlikely (size <= 0))
		return ret_ok;

	/* Get memory
	 */
	if ((cuint_t) free < (size+1)) {
		if (unlikely (realloc_inc_bufsize(buf, size - free)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	
	 */
	memcpy (buf->buf + buf->len, txt, size);

	buf->len += size;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_add_buffer (cherokee_buffer_t *buf, cherokee_buffer_t *buf2)
{
	return cherokee_buffer_add (buf, buf2->buf, buf2->len);
}


ret_t
cherokee_buffer_add_fsize (cherokee_buffer_t *buf, CST_SIZE size)
{
	ret_t       ret;
	int         remain;
	const char  ord[]  = "KMGTPE";
	const char *o      = ord;

	ret = cherokee_buffer_ensure_size (buf, buf->len + 8);
	if (unlikely (ret != ret_ok)) 
		return ret;

	if (size < 973)
		return cherokee_buffer_add_ulong10 (buf, (culong_t)size);

	do {
		remain = (int)(size & 1023);
		size >>= 10;
		if (size >= 973) {
			++o;
			continue;
		}
		if (size < 9 || (size == 9 && remain < 973)) {
			if ((remain = ((remain * 5) + 256) / 512) >= 10)
				++size, remain = 0;
			return cherokee_buffer_add_va_fixed (buf, "%d.%d%c", (int) size, remain, *o);
		}
		if (remain >= 512)
			++size;
		return cherokee_buffer_add_va_fixed (buf, "%3d%c", (int) size, *o);
	} while (1);
	
	return ret_ok;
}


ret_t
cherokee_buffer_add_long10 (cherokee_buffer_t *buf, clong_t lNum)
{
	culong_t ulNum                 = (culong_t) lNum;
	cuint_t  flgNeg                = 0;
	int      newlen                = 0;
	size_t   i                     = (IOS_NUMBUF - 1);
	char     szOutBuf[IOS_NUMBUF];

	if (lNum < 0L) {
		flgNeg = 1;
		ulNum = -ulNum;
	}

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		szOutBuf[--i] = (char) ((ulNum % 10) + '0');
	}
	while ((ulNum /= 10) != 0);

	/* Set sign in any case
	*/
	szOutBuf[--i] = '-';
	i += (flgNeg ^ 1);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;
	return ret_ok;
}


ret_t
cherokee_buffer_add_llong10 (cherokee_buffer_t *buf, cllong_t lNum)
{
	cullong_t ulNum                 = (cullong_t) lNum;
	cuint_t   flgNeg                = 0;
	int       newlen                = 0;
	size_t    i                     = (IOS_NUMBUF - 1);
	char      szOutBuf[IOS_NUMBUF];

	if (lNum < 0L) {
		flgNeg = 1;
		ulNum = -ulNum;
	}

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		szOutBuf[--i] = (char) ((ulNum % 10) + '0');
	}
	while ((ulNum /= 10) != 0);

	/* Set sign in any case
	*/
	szOutBuf[--i] = '-';
	i += (flgNeg ^ 1);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;

	return ret_ok;
}


ret_t
cherokee_buffer_add_ulong10 (cherokee_buffer_t *buf, culong_t ulNum)
{
	int     newlen               = 0;
	size_t  i                    = (IOS_NUMBUF - 1);
	char    szOutBuf[IOS_NUMBUF];

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		szOutBuf[--i] = (char) ((ulNum % 10) + '0');
	}
	while ((ulNum /= 10) != 0);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;

	return ret_ok;
}


ret_t
cherokee_buffer_add_ullong10 (cherokee_buffer_t *buf, cullong_t ulNum)
{
	int     newlen               = 0;
	size_t  i                    = (IOS_NUMBUF - 1);
	char    szOutBuf[IOS_NUMBUF];

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		szOutBuf[--i] = (char) ((ulNum % 10) + '0');
	}
	while ((ulNum /= 10) != 0);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;

	return ret_ok;
}


/*
** Add a number in hexadecimal format to (buf).
*/
ret_t
cherokee_buffer_add_ulong16 (cherokee_buffer_t *buf, culong_t ulNum)
{
	size_t  i                     = (IOS_NUMBUF - 1);
	int     ival                  = 0;
	int     newlen                = 0;
	char    szOutBuf[IOS_NUMBUF];

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		ival = (int) (ulNum & 0xF);
		szOutBuf[--i] = (char) TO_HEX(ival);
	}
	while ((ulNum >>= 4) != 0);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;

	return ret_ok;
}


/*
** Add a number in hexadecimal format to (buf).
*/
ret_t
cherokee_buffer_add_ullong16 (cherokee_buffer_t *buf, cullong_t ulNum)
{
	size_t  i                     = (IOS_NUMBUF - 1);
	int     ival                  = 0;
	int     newlen                = 0;
	char    szOutBuf[IOS_NUMBUF];

	szOutBuf[i] = '\0';

	/* Convert number to string
	 */
	do {
		ival = (int) (ulNum & 0xF);
		szOutBuf[--i] = (char) TO_HEX(ival);
	}
	while ((ulNum >>= 4) != 0);

	/* Verify free space in buffer and if needed then enlarge it.
	*/
	newlen = buf->len + (int) ((IOS_NUMBUF - 1) - i);
	if (unlikely ((cuint_t)newlen >= buf->size)) {
		if (unlikely (realloc_new_bufsize(buf, newlen)) != ret_ok)
			return ret_nomem;
	}

	/* Copy	including '\0'
	 */
	strcpy (buf->buf + buf->len, &szOutBuf[i]);

	buf->len = newlen;

	return ret_ok;
}


ret_t 
cherokee_buffer_add_va_fixed (cherokee_buffer_t *buf, char *format, ...)
{
	int len;
	int size = buf->size - buf->len;	/* final '\0' is always available */
	va_list ap;

	/* Test for minimum buffer size.
	 */
	if (size < 1)
		return ret_error;

	/* Format the string into the buffer.
	 * NOTE: len does NOT include '\0', size includes '\0' (len + 1)
	 */
	va_start (ap, format);
	len = vsnprintf (buf->buf + buf->len, size, format, ap);
	va_end (ap);

	if (unlikely (len < 0)) 
		return ret_error;

	/* Don't expand buffer if there is not enough space.
	 */
	if (unlikely ( len >= size )) 
		return ret_error;

	buf->len += len;
	return ret_ok;
}


ret_t 
cherokee_buffer_add_va_list (cherokee_buffer_t *buf, char *format, va_list args)
{
	int len;
	int estimation;
	int size;
	ret_t ret;
	va_list args2;

	va_copy (args2, args);

	/* Estimate resulting formatted string length.
	 */
	estimation = cherokee_estimate_va_length (format, args);
	if (unlikely (estimation) < 0) {
		PRINT_ERROR ("  -> '%s', esti=%d ( < 0)\n", format, estimation);
		return ret_error;
	}

	/* Ensure enough size for buffer.
	 */
	ret = cherokee_buffer_ensure_size (buf, buf->len + estimation + 2);
	if (ret != ret_ok) {
		PRINT_ERROR ("  -> '%s', esti=%d ensure_size=%d failed !\n", format, estimation, buf->len + estimation + 2);
		return ret;
	}

	/* Format the string into the buffer.
	 * NOTE: len does NOT include '\0', size includes '\0' (len + 1)
	 */
	size = buf->size - buf->len;
	if (size < 1) {
		PRINT_ERROR ("  -> '%s', esti=%d size=%d ( < 1)!\n", format, estimation, size);
		return ret_error;
	}
	len = vsnprintf (buf->buf + buf->len, size, format, args2);

#if 0
	if (estimation < len)
		PRINT_ERROR ("  -> '%s' -> '%s', esti=%d real=%d size=%d\n", 
			     format, buf->buf + buf->len, estimation, len, size);
#endif

	if (unlikely (len < 0))
		return ret_error;

	/* At this point buf-size is always greater than buf-len, thus size > 0.
	 */
	if (len >= size) {
		PRINT_ERROR ("Failed estimation=%d, needed=%d available size=%d: %s\n", 
			     estimation, len, size, format);

		cherokee_buffer_ensure_size (buf, buf->len + len + 2);
		size = buf->size - buf->len;
		len = vsnprintf (buf->buf + buf->len, size, format, args2);

		if (unlikely (len < 0)) 
			return ret_error;

		if (unlikely (len >= size)) 
			return ret_error;
	}

	buf->len += len;
	return ret_ok;
}


ret_t 
cherokee_buffer_add_va (cherokee_buffer_t *buf, char *format, ...)
{
	ret_t   ret;
	va_list ap;

	va_start (ap, format);
	ret = cherokee_buffer_add_va_list (buf, format, ap);
	va_end (ap);

	return ret;
}


ret_t
cherokee_buffer_add_char (cherokee_buffer_t *buf, char c)
{	   
	/* Add char (fast path)
	 */
	if (likely (buf->len + 1 < buf->size)) {
		buf->buf[buf->len++] = c;
		buf->buf[buf->len] = '\0';
		return ret_ok;
	}

	/* Get memory
	 */
	if (unlikely (realloc_inc_bufsize(buf, 1)) != ret_ok)
		return ret_nomem;

	/* Add char
	 */
	buf->buf[buf->len++] = c;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_add_char_n (cherokee_buffer_t *buf, char c, int num)
{
	int free = buf->size - buf->len;

	if (num <= 0)
		return ret_ok;

	/* Get memory
	 */
	if (free < (num+1)) {
		if (unlikely (realloc_inc_bufsize(buf, num - free)) != ret_ok)
			return ret_nomem;
	}

	memset (buf->buf+buf->len, c, num);
	buf->len += num;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t
cherokee_buffer_prepend (cherokee_buffer_t *buf, char *txt, size_t size)
{
	int free = buf->size - buf->len;

	/* Get memory
	 */
	if ((cuint_t) free < (size+1)) {
		if (unlikely (realloc_inc_bufsize(buf, size - free)) != ret_ok)
			return ret_nomem;
	}

	memmove (buf->buf+size, buf->buf, buf->len);

	memcpy (buf->buf, txt, size);
	buf->len += size;
	buf->buf[buf->len] = '\0';
 
	return ret_ok;
}


int   
cherokee_buffer_is_ending (cherokee_buffer_t *buf, char c)
{
	if (cherokee_buffer_is_empty(buf))
		return 0;

	return (buf->buf[buf->len - 1] == c);
}


ret_t
cherokee_buffer_move_to_begin (cherokee_buffer_t *buf, cuint_t pos)
{
	if (pos >= buf->len) {
		cherokee_buffer_clean(buf);
		return ret_ok;
	}

	/* At this point: 0 < pos < buf->len 
	 */
	memmove (buf->buf, buf->buf+pos, (buf->len - pos) + 1);
	buf->len -= pos;

#if 0
	if (strlen(buf->buf) != buf->len) {
		PRINT_ERROR ("ERROR: cherokee_buffer_move_to_begin(): strlen=%d buf->len=%d\n", 
			     strlen(buf->buf), buf->len);
	}
#endif

	return ret_ok;
}


/*
 * Ensure there is enough (addlen) free space left in the buffer.
 */
ret_t
cherokee_buffer_ensure_addlen (cherokee_buffer_t *buf, size_t addlen)
{
	if (buf->len + addlen < buf->size)
		return ret_ok;

	return cherokee_buffer_ensure_size (buf, ((size_t)buf->len + addlen + 1));
}


ret_t
cherokee_buffer_ensure_size (cherokee_buffer_t *buf, size_t size)
{
	char *pbuf;

	/* Maybe it doesn't need it
	 * if buf->size == 0 and size == 0 then buf can be NULL.
	 */
	if (size <= buf->size) 
		return ret_ok;

	/* If it is a new buffer, take memory and return
	 */
	if (buf->buf == NULL) {
		buf->buf = (char *) malloc (size);
		if (unlikely (buf->buf == NULL))
			return ret_nomem;
		buf->size = size;
		return ret_ok;
	}

	/* It already has memory, but it needs more..
	 */
	pbuf = (char *) realloc(buf->buf, size);
	if (unlikely (pbuf == NULL))
		return ret_nomem;

	buf->buf = pbuf;
	buf->size = size;

	return ret_ok;
}


ret_t 
cherokee_buffer_drop_ending (cherokee_buffer_t *buffer, cuint_t num_chars)
{
	int num;

	if (buffer->buf == NULL) {
		return ret_ok;
	}

	num = MIN (num_chars, buffer->len);

	buffer->buf[buffer->len - num] = '\0';
	buffer->len -= num;

	return ret_ok;
}


ret_t
cherokee_buffer_swap_chars (cherokee_buffer_t *buffer, char a, char b)
{
	cuint_t i;

	if (buffer->buf == NULL) {
		return ret_ok;
	}

	for (i=0; i < buffer->len; i++) {
		if (buffer->buf[i] == a) {
			buffer->buf[i] = b;
		}
	}

	return ret_ok;
}


ret_t 
cherokee_buffer_remove_dups (cherokee_buffer_t *buffer, char c)
{
	char       *a      = buffer->buf;
	const char *end    = buffer->buf + buffer->len;
	cuint_t     offset = 0;

	if (buffer->len < 2) {
		return ret_ok;
	}

	do {
		if ((*a == c) && (a[offset+1] == c)) {
			offset++;
			continue;
		}
		
		a++;
		*a = a[offset];

	} while ((a < end) && (offset+1 < buffer->len));

	buffer->len -= offset;
	buffer->buf[buffer->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_remove_string (cherokee_buffer_t *buf, char *string, int string_len)
{
	char *tmp;
	int   offset;

	if (buf->len <= 0) {
		return ret_ok;
	}

	while ((tmp = strstr (buf->buf, string)) != NULL) {
		offset = tmp - buf->buf;
		memmove (tmp, tmp+string_len, buf->len - (offset+string_len) +1);
		buf->len -= string_len;
	}

	return ret_ok;
}


ret_t 
cherokee_buffer_remove_chunk (cherokee_buffer_t *buf, cuint_t from, cuint_t len)
{
	char *end;
	char *begin;

	if (len == buf->len) {
		cherokee_buffer_clean (buf);
		return ret_ok;
	}

	begin = buf->buf + from;
	end   = begin + len;

	memmove (begin, end, ((buf->buf + buf->len) - end) + 1);
	buf->len -= len;

	return ret_ok;
}


cint_t
cherokee_buffer_cmp_buf (cherokee_buffer_t *A, cherokee_buffer_t *B)
{
	if (A->len > B->len)
		return A->len - B->len;
	else if (B->len > A->len)
		return - (B->len - A->len);

	return strncmp (A->buf, B->buf, B->len);
}

cint_t
cherokee_buffer_cmp (cherokee_buffer_t *buf, char *txt, cuint_t txt_len)
{
	cherokee_buffer_t tmp;
	cherokee_buffer_fake (&tmp, txt, txt_len);
	return cherokee_buffer_cmp_buf (buf, &tmp);
}


cint_t
cherokee_buffer_case_cmp_buf (cherokee_buffer_t *A, cherokee_buffer_t *B)
{
	if (A->len > B->len)
		return A->len - B->len;
	else if (B->len > A->len)
		return - (B->len - A->len);

	return strncasecmp (A->buf, B->buf, B->len);
}

cint_t
cherokee_buffer_case_cmp (cherokee_buffer_t *buf, char *txt, cuint_t txt_len)
{
	cherokee_buffer_t tmp;
	cherokee_buffer_fake (&tmp, txt, txt_len);
	return cherokee_buffer_case_cmp_buf (buf, &tmp);
}


size_t
cherokee_buffer_cnt_spn (cherokee_buffer_t *buf, cuint_t offset, char *str) 
{
	if (unlikely ((buf->buf == NULL) || (buf->len <= offset)))
		return 0;

	return strspn (buf->buf + offset, str);
}


size_t 
cherokee_buffer_cnt_cspn (cherokee_buffer_t *buf, cuint_t offset, char *str) 
{
	if (unlikely ((buf->buf == NULL) || (buf->len <= offset)))
		return 0;

	return strcspn (buf->buf + offset, str);
}


crc_t 
cherokee_buffer_crc32 (cherokee_buffer_t *buf)
{
	return crc32_sz (buf->buf, buf->len);
}


ret_t 
cherokee_buffer_read_file (cherokee_buffer_t *buf, char *filename)
{
	int r, f;
	ret_t ret;
	struct stat info;

	/* Stat() the file
	 */
	r = stat (filename, &info);
	if (r != 0)
		return ret_error;

	/* Is a regular file?
	 */
	if (S_ISREG(info.st_mode) == 0)
		return ret_error;

	/* Maybe get memory
	 */
	ret = cherokee_buffer_ensure_size (buf, buf->len + info.st_size + 1);
	if (unlikely (ret != ret_ok))
		return ret;

	/* Open the file
	 */
	f = open (filename, CHE_O_READ);
	if (f < 0)
		return ret_error;

	/* Read the content
	 */
	r = read (f, buf->buf + buf->len, info.st_size);
	if (r < 0) {
		buf->buf[buf->len] = '\0';

		close(f);
		return ret_error;
	}

	/* Close it and exit
	 */
	close(f);

	buf->len += r;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_read_from_fd (cherokee_buffer_t *buf, int fd, size_t size, size_t *ret_size)
{
	int  len;

	/* Ensure there is enough space in buffer
	 * NOTE: usually the caller should have already allocated
	 *       enough space for the buffer, so this is a security measure
	 */
	cherokee_buffer_ensure_addlen(buf, size);

	/* Read data at the end of the buffer
	 */
	len = read (fd, &(buf->buf[buf->len]), size);
	if (len < 0) {
		/* On error
		 */
		switch (errno) {
		case EINTR:
		case EAGAIN:
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
		case EWOULDBLOCK:
#endif
			return ret_eagain;
		case EPIPE:
		case EBADF:
		case ECONNRESET:
			return ret_eof;
		case EIO:
			return ret_error;
		}

		PRINT_ERRNO (errno, "read(%d, %u,..): '${errno}'", fd, size);
		return ret_error;
	}
	else if (len == 0) {
		/* On EOF
		 */
		return ret_eof;
	}

	/* Add read length, terminate buffer and return
	 */
	*ret_size = len;
	buf->len += len;

	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_multiply (cherokee_buffer_t *buf, int num)
{
	int i, initial_size;

	initial_size = buf->len;
	cherokee_buffer_ensure_size (buf, buf->len * num + 1);

	for (i=0; i<num; i++) {
		cherokee_buffer_add (buf, buf->buf, initial_size);
	}

	return ret_ok;
}


ret_t
cherokee_buffer_print_debug (cherokee_buffer_t *buf, int len)
{
	int            i, length;
	char           text[67];
	unsigned char  tmp;
	char          *hex_text   = NULL;
	char          *ascii_text = NULL;

	if ((len == -1) || (buf->len <= (cuint_t)len)) {
		length = buf->len;
	} else {
		length = len;
	}

	if (length <= 0)
		return ret_ok;

	memset(text, 0, 67);
	for (i=0; i < length; i++) {
		if (i%16 == 0) {
			if (text[0] != 0){
				printf ("%s%s", text, CRLF);
			}
			sprintf (text, "%08x%57c", i, ' ');
			hex_text = text + 9;
			ascii_text = text + 49;
		}

		tmp = buf->buf[i];
		sprintf (hex_text, "%02x",  tmp & 0xFF);
		hex_text += 2;
		*hex_text = ' ';
		if ((i+1)%2 == 0) {
			hex_text++;
		}

		if ((tmp > ' ') &&  (tmp < 128))
			*ascii_text = tmp;
		else
			*ascii_text = '.';
		ascii_text += 1;
	}
	printf ("%s%s", text, CRLF);
	fflush(stdout);

	return ret_ok;
}


/*
 * Unescape a string that may have escaped characters %xx
 * where xx is the hexadecimal number equal to the character ascii value.
 */
ret_t
cherokee_buffer_unescape_uri (cherokee_buffer_t *buffer)
{
	static const char hex2dec_tab[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 00-0F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 10-1F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 20-2F */
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,  /* 30-3F */
		0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 40-4F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 50-5F */
		0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 60-6F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 70-7F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 80-8F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 90-9F */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* A0-AF */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* B0-BF */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* C0-CF */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* D0-DF */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* E0-EF */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   /* F0-FF */
	};

	char *psrc;
	char *ptgt;
	int   len;

#define hex2dec_m(c)	   ( (int) hex2dec_tab[ ( (unsigned char )(c) ) ] )
#define hex2dec_m2(c1, c2) ( hex2dec_m(c1) * 16 + hex2dec_m(c2) )

	TRACE(ENTRIES, "Prev: %s\n", buffer->buf);

	if (unlikely (buffer->buf == NULL))
		return ret_error;

	/* Verify string termination,
	 * we assume there are no '\0' inside buffer.
	 */
	if (buffer->buf[buffer->len] != '\0')
		buffer->buf[buffer->len]  = '\0';

	/* Verify if unescaping is needed.
	 */
	if ((psrc = strchr (buffer->buf, '%')) == NULL)
		return ret_ok;

	/* Yes, unescape string.
	 */
	len = buffer->len;
	for (ptgt = psrc; *psrc != '\0'; ++ptgt, ++psrc) {
		if (psrc[0] != '%' ||
		    !isxdigit(psrc[1]) || !isxdigit(psrc[2])) {
			*ptgt = *psrc;
			continue;
		}
		/* Escape sequence %xx
		 */
		if (likely ((*ptgt = hex2dec_m2(psrc[1], psrc[2])) != '\0')) {
			psrc += 2;
			len  -= 2;
			continue;
		}
		/* Replace null bytes (%00) with
		 * spaces, to prevent attacks
		 */
		*ptgt = ' ';
		psrc += 2;
		len  -= 2;
	}
	*ptgt = '\0';
	buffer->len = len;

#undef hex2dec_m2
#undef hex2dec_m

	TRACE(ENTRIES, "Post: %s\n", buffer->buf);
	return ret_ok;
}


ret_t 
cherokee_buffer_add_escape_html (cherokee_buffer_t *buf, cherokee_buffer_t *src)
{
	ret_t   ret;
	size_t  len0 = 0;
	size_t  extra = 0;
	char   *p0, *p1, *p2;

	/* Verify that source string is not empty.
	 */
	if (unlikely (src->buf == NULL))
		return ret_error;

	/* Verify string termination,
	 * we assume there are no '\0' inside buffer.
	 */
	if (src->buf[src->len] != '\0')
		src->buf[src->len]  = '\0';

	/* Verify if string has to be escaped.
	 */
	if ((p0 = strpbrk (src->buf, "<>&\"")) == NULL) {
		/* No escape found, simply add src to buf.
		 */
		return cherokee_buffer_add_buffer (buf, src);
	}

	/* Count extra characters
	 */
	for (p1 = p0; *p1 != '\0'; ++p1) {
		switch(*p1) {
			case '<':	/* &lt; */
			case '>':	/* &gt; */
				extra += 3;
				continue;
			case '&':	/* &amp; */
				extra += 4;
				continue;
			case '"':	/* &quot; */
				extra += 5;
				continue;
			default:
				continue;
		}
	}

	/* Verify there are no embedded '\0'.
	 */
	if ( (cuint_t)(p1 - src->buf) != src->len)
		return ret_error;

	/* Ensure there is proper buffer size.
	 */
	ret = cherokee_buffer_ensure_addlen (buf, src->len + extra + 1);
	if (ret != ret_ok)
		return ret;

	/* Escape and copy data to destination buffer.
	 */
	if (p0 != src->buf) {
		len0 = (size_t) (p0 - src->buf);
		memcpy (&buf->buf[buf->len], src->buf, len0);
	}

	p2 = &buf->buf[buf->len + len0];

	for (p1 = p0; *p1 != '\0'; ++p1) {
		switch (*p1) {
		case '<':
			memcpy (p2, "&lt;", 4);
			p2 += 4;
			continue;

		case '>':
			memcpy (p2, "&gt;", 4);
			p2 += 4;
			continue;

		case '&':
			memcpy (p2, "&amp;", 5);
			p2 += 5;
			continue;

		case '"':
			memcpy (p2, "&quot;", 6);
			p2 += 6;
			continue;

		default:
			*p2++ = *p1;
			continue;
		}
	}
	
	/* Set the new length
	 */
	buf->len += src->len + extra;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t 
cherokee_buffer_escape_html (cherokee_buffer_t *buf, cherokee_buffer_t *src)
{
	cherokee_buffer_clean (buf);
	return cherokee_buffer_add_escape_html (buf, src);
}


ret_t 
cherokee_buffer_decode_base64 (cherokee_buffer_t *buf)
{
	cuint_t  i;
	char     space[128];
	int      space_idx = 0;
	int      phase     = 0;
	int      d, prev_d = 0;
	int      buf_pos   = 0;

	/* Base-64 decoding: This represents binary data as printable
	 * ASCII characters. Three 8-bit binary bytes are turned into
	 * four 6-bit values, like so:
	 *	
	 *   [11111111]  [22222222]  [33333333]
	 *   [111111] [112222] [222233] [333333]
	 *
	 * Then the 6-bit values are represented using the characters
	 * "A-Za-z0-9+/".
	 */

	static const signed char
		b64_decode_tab[256] = {
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
			52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
			-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
			15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
			-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
			41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
		};

	for (i=0; i < buf->len; i++) {
		d = b64_decode_tab[(int) buf->buf[i]];
		if (d != -1) {
			switch (phase) {
			case 0:
				++phase;
				break;
			case 1:
				space[space_idx++] = (( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ));
				++phase;
				break;
			case 2:
				space[space_idx++] = (( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ));
				++phase;
				break;
			case 3:
				space[space_idx++] = (( ( prev_d & 0x03 ) << 6 ) | d );
				phase = 0;
				break;
			}
			prev_d = d;
		} 

		if (space_idx == 127) {
			memcpy (buf->buf + buf_pos, space, 127);
			buf_pos += 127;
			space_idx = 0;
		}
	}

	space[space_idx]='\0';

	memcpy (buf->buf + buf_pos, space, space_idx+1);
	buf->len = buf_pos + space_idx;
	
	return ret_ok;
}


/* Encode base64 from source (buf) to destination (encoded).
 * NOTE: resulting (encoded) content is always longer than source (buf).
 * Source (buf) is not touched (rewritten or reallocated).
 */
ret_t 
cherokee_buffer_encode_base64 (cherokee_buffer_t *buf, cherokee_buffer_t *encoded)
{
	cuchar_t         *in;
	cuchar_t         *out;
	ret_t             ret;
	cuint_t           i, j;
	cuint_t           inlen   = buf->len;

	static const char base64tab[]=
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	/* Get memory
	 */
	ret = cherokee_buffer_ensure_size (encoded, (buf->len+4)*4/3 + 1);
	if (unlikely (ret != ret_ok))
		return ret;

	/* Cleanup destination buffer
	 */
	cherokee_buffer_clean (encoded);

	/* Encode source to destination
	 */
	in  = (cuchar_t *) buf->buf;
	out = (cuchar_t *) encoded->buf;

	for (i=0, j=0; i < inlen; i += 3) {
		int     a=0,b=0,c=0;
		int     d, e, f, g;

		a=in[i];
		b= i+1 < inlen ? in[i+1]:0;
		c= i+2 < inlen ? in[i+2]:0;

		d = base64tab [a >> 2 ];
		e = base64tab [((a & 3 ) << 4) | (b >> 4)];
		f = base64tab [((b & 15) << 2) | (c >> 6)];
		g = base64tab [c & 63 ];

		if (i + 1 >= inlen)
			f = '=';
		if (i + 2 >= inlen)
			g = '=';

		out[j++] = d;
		out[j++] = e;
		out[j++] = f;
		out[j++] = g;
	}

	out[j]  = '\0';
	encoded->len = j;

	return ret_ok;
}


/* Documentation: 
 * RFC 1321, `The MD5 Message-Digest Algorithm'
 * http://www.alobbs.com/modules.php?op=modload&name=rfc&file=index&content_file=rfc1321.php
 *
 * The MD5 message-digest algorithm takes as input a message of
 * arbitrary length and produces as output a 128-bit "fingerprint" or
 * "message digest" of the input. 
 */

ret_t 
cherokee_buffer_encode_md5_digest (cherokee_buffer_t *buf)
{
	int i;
	struct MD5Context context;
	unsigned char digest[16];

	MD5Init (&context);
	MD5Update (&context, (md5byte *)buf->buf, buf->len);
	MD5Final (digest, &context);

	cherokee_buffer_ensure_size (buf, 34);

	for (i = 0; i < 16; ++i) {
		int tmp;

		tmp = ((digest[i] >> 4) & 0xf);
		buf->buf[i*2] = TO_HEX(tmp);

		tmp = (digest[i] & 0xf);
		buf->buf[(i*2)+1] = TO_HEX(tmp);
	}
	buf->buf[32] = '\0';
	buf->len = 32;

	return ret_ok;
}


ret_t 
cherokee_buffer_encode_md5 (cherokee_buffer_t *buf, cherokee_buffer_t *encoded)
{
	struct MD5Context context;

	cherokee_buffer_ensure_size (encoded, 17);

	MD5Init (&context);
	MD5Update (&context, (md5byte *)buf->buf, buf->len);
	MD5Final ((unsigned char *)encoded->buf, &context);

	encoded->buf[16] = '\0';
	encoded->len = 16;

	return ret_ok;
}


/* Encode sha1, source buffer (buf) is not touched,
 * whereas destination buffer (encoded) is overwritten
 * but possibly not reallocated.
 */
ret_t 
cherokee_buffer_encode_sha1 (cherokee_buffer_t *buf, cherokee_buffer_t *encoded)
{
	SHA_INFO sha1;

	sha_init (&sha1);
	sha_update (&sha1, (unsigned char*) buf->buf, buf->len);

	cherokee_buffer_ensure_size (encoded, SHA1_DIGEST_SIZE + 1);
	sha_final (&sha1, (unsigned char *) encoded->buf);

	encoded->len = SHA1_DIGEST_SIZE;
	encoded->buf[encoded->len] = '\0';

	return ret_ok;
}


/* Encode sha1 in base64, both source (buf) and destination (encoded)
 * buffers are overwritten, but possibly not reallocated.
 */
ret_t 
cherokee_buffer_encode_sha1_base64 (cherokee_buffer_t *buf, cherokee_buffer_t *encoded) 
{
	/* Prepare destination buffer
	 */
	cherokee_buffer_ensure_size (encoded, (SHA1_DIGEST_SIZE * 2) + 1);	
	cherokee_buffer_clean (encoded);

	/* Encode sha1 + base64
	 */
	cherokee_buffer_encode_sha1 (buf, encoded);
	cherokee_buffer_encode_base64 (encoded, buf);

	/* Copy result to destination buffer
	 */
	cherokee_buffer_clean (encoded);
	cherokee_buffer_add_buffer (encoded, buf);

	return ret_ok;
}


/* Encode in hexadecimal characters, source buffer (buf) is not touched,
 * whereas destination buffer (encoded) is overwritten
 * but possibly not reallocated.
 */
ret_t 
cherokee_buffer_encode_hex (cherokee_buffer_t *buf, cherokee_buffer_t *encoded)
{
	cuchar_t        *in;
	cuchar_t        *out;
	cuint_t         j;
	cuint_t         i;
	cuint_t         inlen = buf->len;

	/* Prepare destination buffer
	 */
	cherokee_buffer_ensure_size (encoded, (inlen * 2 + 1));	
	cherokee_buffer_clean (encoded);

	/* Encode source to destination
	 */
	in  = (cuchar_t *) buf->buf;
	out = (cuchar_t *) encoded->buf;

	for (i = 0; i != inlen; ++i) {
		j = ( (*in >> 4) & 0xf );
		*out++ = (cuchar_t) TO_HEX(j);

		j =   (*in++ & 0xf);
		*out++ = (cuchar_t) TO_HEX(j);
	}

	*out = '\0';
	encoded->len = (int) (inlen * 2);

	return ret_ok;
}


ret_t 
cherokee_buffer_decode_hex (cherokee_buffer_t *buf)
{
	cuint_t i;

	static char hex_to_bin [128] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
		 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,    /*   0..9     */
		-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   A..F     */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
		-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   a..f     */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };  /*            */


	for (i=0; i<buf->len/2; i++) {
		/* It uses << 1 rather than * 2
		 */
		cint_t b1 = buf->buf[(i << 1)] & 127;
		cint_t b2 = buf->buf[(i << 1) + 1] & 127;

 		b1 = hex_to_bin[b1];
 		b2 = hex_to_bin[b2];

		if ((b1 == -1) || (b2 == -1))
			break;

		buf->buf[i] = (((b1 << 4) & 0xF0) | (b2 & 0x0F));
	}

	buf->len /= 2;
	buf->buf[buf->len] = '\0';

	return ret_ok;
}


ret_t
cherokee_buffer_convert_to_chunked (cherokee_buffer_t *buf)
{
	int  len;
	char tmp[40];

	len = sprintf (tmp, "%x", buf->len);
	if (unlikely (len > 40-3))
		return ret_error;

	tmp[len]   = CHR_CR;
	tmp[len+1] = CHR_LF;
	tmp[len+2] = '\0';

	cherokee_buffer_ensure_size (buf, buf->len + len + 2);
	cherokee_buffer_prepend (buf, tmp, len+2);
	cherokee_buffer_add_str (buf, CRLF);

	return ret_ok;
}


ret_t 
cherokee_buffer_add_chunked (cherokee_buffer_t *buf, char *txt, size_t size)
{
	ret_t ret;

        /* Chunk format (simplified):
	 *
	 * <HEX SIZE> [ chunk extension ] CRLF
	 * <DATA> CRLF
	 */

	ret = cherokee_buffer_add_ulong16 (buf, (culong_t)size);
	if (unlikely (ret < ret_ok))
		return ret_ok;

	ret = cherokee_buffer_add_str (buf, CRLF);
	if (unlikely (ret < ret_ok))
		return ret_ok;

	ret = cherokee_buffer_add (buf, txt, size);
	if (unlikely (ret < ret_ok))
		return ret_ok;

	return cherokee_buffer_add_str (buf, CRLF);
}


ret_t 
cherokee_buffer_add_buffer_chunked (cherokee_buffer_t *buf, cherokee_buffer_t *buf2)
{
	return cherokee_buffer_add_chunked (buf, buf2->buf, buf2->len);
}


char  
cherokee_buffer_end_char (cherokee_buffer_t *buf)
{
	if ((buf->buf == NULL) || (buf->len <= 0))
		return '\0';

	return buf->buf[buf->len-1];
}


ret_t 
cherokee_buffer_replace_string (cherokee_buffer_t *buf, 
				char *substring,   int substring_length, 
				char *replacement, int replacement_length)
{
	int         remaining_length;
	int         result_length;
	char       *result;
	char       *result_position;
	const char *p;
	const char *substring_position;

	/* Verify formal parameters
	 * (those which are not tested would raise a segment violation).
	 */
	if (buf == NULL || buf->buf == NULL ||
	    substring == NULL || substring_length < 1 ||
		replacement == NULL || replacement_length < 0)
		return ret_deny;

	/* Calculate the new size
	 */
	result_length = buf->len;
	for (p = buf->buf; ; p = substring_position + substring_length) {
		substring_position = strstr (p, substring);

		if (substring_position == NULL)
			break;

		result_length += (replacement_length - substring_length);
	}
	
	/* If no substring has been found, then return now.
	 */
	if (p == buf->buf)
		return ret_ok;

	/* If resulting length is zero, then return now.
	 */
	if (result_length < 1) {
		buf->buf[0] = '\0';
		buf->len = 0;
		return ret_ok;
	}

	/* Take the new memory chunk
	 */
	result = (char *) malloc (result_length + 1);
	if (unlikely (result == NULL))
		return ret_nomem;

	/* Build the new string
	 */
	result_position = result;

	for (p = buf->buf; ; p = substring_position + substring_length) {
		substring_position = strstr (p, substring);

		if (substring_position == NULL) {
			remaining_length = strlen (p);
			memcpy (result_position, p, remaining_length);
			result_position += remaining_length;
			break;
		}
		memcpy (result_position, p, substring_position - p);
		result_position += (substring_position - p);

		memcpy (result_position, replacement, replacement_length);
		result_position += replacement_length;
	}	
	*result_position = '\0';

	/* Change the internal buffer content
	 */
	free (buf->buf);

	buf->buf  = result;
	buf->len  = result_length;
	buf->size = result_length + 1;

	return ret_ok;	
}


/* Substitute (substring)s found in (bufsrc) with (replacement)
 * and writes the resulting content to (bufdst).
 * NOTE: (bufdst) is written only if at least one (substring) instance
 *       is found in (bufsrc); in that case return value is ret_ok;
 *       if (substring) is NOT found in (bufsrc) then nothing is done
 *       in order to avoid an unnecessary copy of data.
 * Returns:
 *  ret_ok          bufdst has been written with the substitution string(s)
 *  ret_not_found   substring not found in bufsrc
 *  ret_deny        bad formal parameters
 *  ret_xxx         fatal error (failed allocation, etc.)
 */
ret_t 
cherokee_buffer_substitute_string (cherokee_buffer_t *bufsrc, 
				   cherokee_buffer_t *bufdst,
				   char *substring,   int substring_length, 
				   char *replacement, int replacement_length)
{
	ret_t       ret;
	int         remaining_length;
	int         result_length;
	char       *result_position;
	const char *p;
	const char *substring_position;

	/* Verify formal parameters
	 * (those which are not tested would raise a segment violation).
	 */
	if (bufsrc->buf == NULL ||
	    bufdst->buf == NULL ||
	    substring == NULL || substring_length < 1 ||
	    replacement == NULL || replacement_length < 0)
		return ret_deny;

	/* Clean / reset destination buffer.
	 */
	bufdst->buf[0] = '\0';
	bufdst->len = 0;

	/* Calculate the new size
	 */
	result_length = bufsrc->len;
	for (p = bufsrc->buf; ; p = substring_position + substring_length) {
		substring_position = strstr (p, substring);

		if (substring_position == NULL)
			break;

		result_length += (replacement_length - substring_length);
	}
	
	/* If no substring has been found, then return now.
	 */
	if (p == bufsrc->buf)
		return ret_not_found;

	/* If resulting length is zero, then return now.
	 */
	if (result_length < 1) {
		return ret_ok;
	}

	/* Preset size of destination buffer.
	 */
	ret = cherokee_buffer_ensure_size(bufdst, result_length + 2);

	/* Build the new string
	 */
	result_position = bufdst->buf;

	for (p = bufsrc->buf; ; p = substring_position + substring_length) {
		substring_position = strstr (p, substring);

		if (substring_position == NULL) {
			remaining_length = (int) (&(bufsrc->buf[bufsrc->len]) - p);
			memcpy (result_position, p, remaining_length);
			result_position += remaining_length;
			break;
		}
		memcpy (result_position, p, substring_position - p);
		result_position += (int) (substring_position - p);

		memcpy (result_position, replacement, replacement_length);
		result_position += replacement_length;
	}	

	/* Terminate the destination buffer
	 */
	*result_position = '\0';
	bufdst->len  = result_length;

	return ret_ok;	
}


ret_t 
cherokee_buffer_add_comma_marks (cherokee_buffer_t *buf)
{
	cuint_t  off, num, i;
	char    *p;

	if ((buf->buf == NULL) || (buf->len <= 3))
		return ret_ok;

	num = buf->len / 3;
	off = buf->len % 3;

	cherokee_buffer_ensure_size (buf, buf->len + num + 2);

	if (off == 0) {
		p = buf->buf + 3;
		num--;
	} else {
		p = buf->buf + off;
	}

	for (i = 0; i < num; i++) {
		int len = (buf->buf + buf->len) - p;
		memmove(p+1, p, len);
		*p = ',';
		p +=4;
		buf->len++;
	}

	buf->buf[buf->len] = '\0';
	return ret_ok;
}


ret_t 
cherokee_buffer_trim (cherokee_buffer_t *buf)
{
	cuint_t s, e;
	cuint_t len;

	if (buf->len <= 0)
		return ret_ok;

	for (s=0; s < buf->len; s++) {
		char c = buf->buf[s];

		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
	}

	for (e=0; e < (buf->len - s); e++) {
		char c = buf->buf[buf->len-(e+1)];

		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;		
	}

	len = buf->len - (s + e);

	memmove (buf->buf, buf->buf+s, len);

	buf->len = len;
	buf->buf[len] = '\0';

	return ret_ok;
}


ret_t
cherokee_buffer_to_lowcase (cherokee_buffer_t *buf)
{
	char    c;
	cuint_t i;

	for (i=0; i<buf->len; i++) {
		c = buf->buf[i];
		if ((c >= 'A') && (c <= 'Z')) {
			buf->buf[i] = c + ('a'-'A');
		}
	}

	return ret_ok;
}
