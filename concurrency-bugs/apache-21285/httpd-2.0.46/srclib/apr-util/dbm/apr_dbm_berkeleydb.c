/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "apr_strings.h"
#define APR_WANT_MEMFUNC
#include "apr_want.h"
#define APU_DBM_BERKELEYDB_PRIVATE 1

#if APR_HAVE_STDLIB_H
#include <stdlib.h> /* for abort() */
#endif

#include "apu.h"

#if APU_HAVE_DB 
#include "apr_dbm_private.h"

/*
 * We pick up all varieties of Berkeley DB through db.h (included through
 * apu_select_dbm.h). This code has been compiled/tested against DB1,
 * DB_185, DB2, DB3, and DB4.
 */

#if   defined(DB_VERSION_MAJOR) && (DB_VERSION_MAJOR == 4)
/* We will treat anything greater than 4.1 as DB4.
 * We can treat 4.0 as DB3.
 */
#if   defined(DB_VERSION_MINOR) && (DB_VERSION_MINOR >= 1)
#define DB_VER 4
#else
#define DB_VER 3
#endif
#elif defined(DB_VERSION_MAJOR) && (DB_VERSION_MAJOR == 3)
#define DB_VER 3
#elif defined(DB_VERSION_MAJOR) && (DB_VERSION_MAJOR == 2)
#define DB_VER 2
#else
#define DB_VER 1
#endif

typedef struct {
    DB *bdb;
#if DB_VER != 1
    DBC *curs;
#endif
} real_file_t;


#if DB_VER == 1
#define TXN_ARG
#else
#define TXN_ARG NULL,
#endif

#define GET_BDB(f)      (((real_file_t *)(f))->bdb)

#define do_fetch(bdb, k, v)     ((*(bdb)->get)(bdb, TXN_ARG &(k), &(v), 0))

#if DB_VER == 1
#include <sys/fcntl.h>
#define APR_DBM_DBMODE_RO       O_RDONLY
#define APR_DBM_DBMODE_RW       O_RDWR
#define APR_DBM_DBMODE_RWCREATE (O_CREAT | O_RDWR)
#define APR_DBM_DBMODE_RWTRUNC  (O_CREAT | O_RDWR | O_TRUNC)
#else
#define APR_DBM_DBMODE_RO       DB_RDONLY
#define APR_DBM_DBMODE_RW       0
#define APR_DBM_DBMODE_RWCREATE DB_CREATE
#define APR_DBM_DBMODE_RWTRUNC  DB_TRUNCATE
#endif /* DBVER == 1 */

/* --------------------------------------------------------------------------
**
** UTILITY FUNCTIONS
*/

/* map a DB error to an apr_status_t */
static apr_status_t db2s(int dberr)
{
    if (dberr != 0) {
        /* ### need to fix this */
        return APR_OS_START_USEERR + dberr;
    }

    return APR_SUCCESS;
}


static apr_status_t set_error(apr_dbm_t *dbm, apr_status_t dbm_said)
{
    apr_status_t rv = APR_SUCCESS;

    /* ### ignore whatever the DBM said (dbm_said); ask it explicitly */

    if (dbm_said == APR_SUCCESS) {
        dbm->errcode = 0;
        dbm->errmsg = NULL;
    }
    else {
        /* ### need to fix. dberr was tossed in db2s(). */
        /* ### use db_strerror() */
        dbm->errcode = dbm_said;
#if DB_VER == 1 || DB_VER == 2
        dbm->errmsg = NULL;
#else
        dbm->errmsg = db_strerror(dbm_said - APR_OS_START_USEERR);
#endif
        rv = dbm_said;
    }

    return rv;
}

/* --------------------------------------------------------------------------
**
** DEFINE THE VTABLE FUNCTIONS FOR BERKELEY DB
**
** ### we may need three sets of these: db1, db2, db3
*/

static apr_status_t vt_db_open(apr_dbm_t **pdb, const char *pathname,
                               apr_int32_t mode, apr_fileperms_t perm,
                               apr_pool_t *pool)
{
    real_file_t file;
    int dbmode;

    *pdb = NULL;

    switch (mode) {
    case APR_DBM_READONLY:
        dbmode = APR_DBM_DBMODE_RO;
        break;
    case APR_DBM_READWRITE:
        dbmode = APR_DBM_DBMODE_RW;
        break;
    case APR_DBM_RWCREATE:
        dbmode = APR_DBM_DBMODE_RWCREATE;
        break;
    case APR_DBM_RWTRUNC:
        dbmode = APR_DBM_DBMODE_RWTRUNC;
        break;
    default:
        return APR_EINVAL;
    }

    {
        int dberr;

#if DB_VER >= 3
        if ((dberr = db_create(&file.bdb, NULL, 0)) == 0) {
            if ((dberr = (*file.bdb->open)(file.bdb,
#if DB_VER == 4
                                           NULL,
#endif
                                           pathname, NULL, 
                                           DB_HASH, dbmode, 
                                           apr_posix_perms2mode(perm))) != 0) {
                /* close the DB handler */
                (void) (*file.bdb->close)(file.bdb, 0);
            }
        }
        file.curs = NULL;
#elif DB_VER == 2
        dberr = db_open(pathname, DB_HASH, dbmode, apr_posix_perms2mode(perm),
                        NULL, NULL, &file.bdb);
        file.curs = NULL;
#else
        file.bdb = dbopen(pathname, dbmode, apr_posix_perms2mode(perm),
                          DB_HASH, NULL);
        if (file.bdb == NULL)
            return APR_EGENERAL;      /* ### need a better error */
        dberr = 0;
#endif
        if (dberr != 0)
            return db2s(dberr);
    }

    /* we have an open database... return it */
    *pdb = apr_pcalloc(pool, sizeof(**pdb));
    (*pdb)->pool = pool;
    (*pdb)->type = &apr_dbm_type_db;
    (*pdb)->file = apr_pmemdup(pool, &file, sizeof(file));

    /* ### register a cleanup to close the DBM? */

    return APR_SUCCESS;
}

static void vt_db_close(apr_dbm_t *dbm)
{
    (*GET_BDB(dbm->file)->close)(GET_BDB(dbm->file)
#if DB_VER != 1
                                 , 0
#endif
        );
}

static apr_status_t vt_db_fetch(apr_dbm_t *dbm, apr_datum_t key,
                                apr_datum_t * pvalue)
{
    DBT ckey = { 0 };
    DBT rd = { 0 };
    int dberr;

    ckey.data = key.dptr;
    ckey.size = key.dsize;

    dberr = do_fetch(GET_BDB(dbm->file), ckey, rd);

    /* "not found" is not an error. return zero'd value. */
    if (dberr ==
#if DB_VER == 1
        RET_SPECIAL
#else
        DB_NOTFOUND
#endif
        ) {
        memset(&rd, 0, sizeof(rd));
        dberr = 0;
    }

    pvalue->dptr = rd.data;
    pvalue->dsize = rd.size;

    /* store the error info into DBM, and return a status code. Also, note
       that *pvalue should have been cleared on error. */
    return set_error(dbm, db2s(dberr));
}

static apr_status_t vt_db_store(apr_dbm_t *dbm, apr_datum_t key,
                                apr_datum_t value)
{
    apr_status_t rv;
    DBT ckey = { 0 };
    DBT cvalue = { 0 };

    ckey.data = key.dptr;
    ckey.size = key.dsize;

    cvalue.data = value.dptr;
    cvalue.size = value.dsize;

    rv = db2s((*GET_BDB(dbm->file)->put)(GET_BDB(dbm->file),
                                         TXN_ARG
                                         &ckey,
                                         &cvalue,
                                         0));

    /* store any error info into DBM, and return a status code. */
    return set_error(dbm, rv);
}

static apr_status_t vt_db_del(apr_dbm_t *dbm, apr_datum_t key)
{
    apr_status_t rv;
    DBT ckey = { 0 };

    ckey.data = key.dptr;
    ckey.size = key.dsize;

    rv = db2s((*GET_BDB(dbm->file)->del)(GET_BDB(dbm->file),
                                         TXN_ARG
                                         &ckey,
                                         0));

    /* store any error info into DBM, and return a status code. */
    return set_error(dbm, rv);
}

static int vt_db_exists(apr_dbm_t *dbm, apr_datum_t key)
{
    DBT ckey = { 0 };   /* converted key */
    DBT data = { 0 };
    int dberr;

    ckey.data = key.dptr;
    ckey.size = key.dsize;

    dberr = do_fetch(GET_BDB(dbm->file), ckey, data);

    /* note: the result data is "loaned" to us; we don't need to free it */

    /* DB returns DB_NOTFOUND if it doesn't exist. but we want to say
       that *any* error means it doesn't exist. */
    return dberr == 0;
}

static apr_status_t vt_db_firstkey(apr_dbm_t *dbm, apr_datum_t * pkey)
{
    real_file_t *f = dbm->file;
    DBT first = { 0 };
    DBT data = { 0 };
    int dberr;

#if DB_VER == 1
    dberr = (*f->bdb->seq)(f->bdb, &first, &data, R_FIRST);
#else
    if ((dberr = (*f->bdb->cursor)(f->bdb, NULL, &f->curs
#if DB_VER >= 3 || ((DB_VERSION_MAJOR == 2) && (DB_VERSION_MINOR > 5))
                                   , 0
#endif
             )) == 0) {
        dberr = (*f->curs->c_get)(f->curs, &first, &data, DB_FIRST);
        if (dberr == DB_NOTFOUND) {
            memset(&first, 0, sizeof(first));
            (*f->curs->c_close)(f->curs);
            f->curs = NULL;
            dberr = 0;
        }
    }
#endif

    pkey->dptr = first.data;
    pkey->dsize = first.size;

    /* store any error info into DBM, and return a status code. */
    return set_error(dbm, db2s(dberr));
}

static apr_status_t vt_db_nextkey(apr_dbm_t *dbm, apr_datum_t * pkey)
{
    real_file_t *f = dbm->file;
    DBT ckey = { 0 };
    DBT data = { 0 };
    int dberr;

    ckey.data = pkey->dptr;
    ckey.size = pkey->dsize;

#if DB_VER == 1
    dberr = (*f->bdb->seq)(f->bdb, &ckey, &data, R_NEXT);
    if (dberr == RET_SPECIAL) {
        dberr = 0;
        ckey.data = NULL;
        ckey.size = 0;
    }
#else
    if (f->curs == NULL)
        return APR_EINVAL;

    dberr = (*f->curs->c_get)(f->curs, &ckey, &data, DB_NEXT);
    if (dberr == DB_NOTFOUND) {
        (*f->curs->c_close)(f->curs);
        f->curs = NULL;
        dberr = 0;
        ckey.data = NULL;
        ckey.size = 0;
    }
#endif

    pkey->dptr = ckey.data;
    pkey->dsize = ckey.size;

    /* store any error info into DBM, and return a status code. */
    /* ### or use db2s(dberr) instead of APR_SUCCESS? */
    return set_error(dbm, APR_SUCCESS);
}

static void vt_db_freedatum(apr_dbm_t *dbm, apr_datum_t data)
{
    /* nothing to do */
}

static void vt_db_usednames(apr_pool_t *pool, const char *pathname,
                            const char **used1, const char **used2)
{
    *used1 = apr_pstrdup(pool, pathname);
    *used2 = NULL;
}


APU_DECLARE_DATA const apr_dbm_type_t apr_dbm_type_db = {
    "db",

    vt_db_open,
    vt_db_close,
    vt_db_fetch,
    vt_db_store,
    vt_db_del,
    vt_db_exists,
    vt_db_firstkey,
    vt_db_nextkey,
    vt_db_freedatum,
    vt_db_usednames
};

#endif /* APU_HAVE_DB */