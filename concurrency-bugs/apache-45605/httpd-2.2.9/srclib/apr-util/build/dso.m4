dnl -------------------------------------------------------- -*- autoconf -*-
dnl Licensed to the Apache Software Foundation (ASF) under one or more
dnl contributor license agreements.  See the NOTICE file distributed with
dnl this work for additional information regarding copyright ownership.
dnl The ASF licenses this file to You under the Apache License, Version 2.0
dnl (the "License"); you may not use this file except in compliance with
dnl the License.  You may obtain a copy of the License at
dnl
dnl     http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.

dnl
dnl DSO module
dnl

AC_DEFUN([APU_CHECK_UTIL_DSO], [

  AC_ARG_ENABLE([util-dso], 
     APR_HELP_STRING([--disable-util-dso],
       [disable DSO build of modular components (dbd, ldap)]))

  if test "$enable_util_dso" = "no"; then
     # Statically link the DBD drivers:

     objs=
     test $apu_have_oracle = 1 && objs="$objs dbd/apr_dbd_oracle.lo"
     test $apu_have_pgsql = 1 && objs="$objs dbd/apr_dbd_pgsql.lo"
     test $apu_have_mysql = 1 && objs="$objs dbd/apr_dbd_mysql.lo"
     test $apu_have_sqlite2 = 1 && objs="$objs dbd/apr_dbd_sqlite2.lo"
     test $apu_have_sqlite3 = 1 && objs="$objs dbd/apr_dbd_sqlite3.lo"
     test $apu_have_freetds = 1 && objs="$objs dbd/apr_dbd_freetds.lo"
     test $apu_has_ldap = 1 && objs="$objs ldap/apr_ldap_init.lo"
     test $apu_has_ldap = 1 && objs="$objs ldap/apr_ldap_option.lo"
     test $apu_has_ldap = 1 && objs="$objs ldap/apr_ldap_rebind.lo"
     EXTRA_OBJECTS="$EXTRA_OBJECTS $objs"

     # Use libtool *.la for mysql if available
     if test $apu_have_mysql = 1; then
       for flag in $LDADD_dbd_mysql
       do
         dir=`echo $flag | grep "^-L" | sed s:-L::`
         if test "x$dir" != 'x'; then
           if test -f "$dir/libmysqlclient_r.la"; then
             LDADD_dbd_mysql=$dir/libmysqlclient_r.la
             break
           fi
         fi
       done
     fi

     APRUTIL_LIBS="$APRUTIL_LIBS $LDADD_dbd_pgsql $LDADD_dbd_sqlite2 $LDADD_dbd_sqlite3 $LDADD_dbd_oracle $LDADD_dbd_mysql $LDADD_dbd_freetds"
     APRUTIL_LIBS="$APRUTIL_LIBS $LDADD_ldap"
     APRUTIL_EXPORT_LIBS="$APRUTIL_EXPORT_LIBS $LDADD_dbd_pgsql $LDADD_dbd_sqlite2 $LDADD_dbd_sqlite3 $LDADD_dbd_oracle $LDADD_dbd_mysql $LDADD_dbd_freetds"
     APRUTIL_EXPORT_LIBS="$APRUTIL_EXPORT_LIBS $LDADD_ldap"
  else
     AC_DEFINE([APU_DSO_BUILD], 1, [Define if modular components are built as DSOs])
     
     dsos=
     test $apu_have_oracle = 1 && dsos="$dsos dbd/apr_dbd_oracle.la"
     test $apu_have_pgsql = 1 && dsos="$dsos dbd/apr_dbd_pgsql.la"
     test $apu_have_mysql = 1 && dsos="$dsos dbd/apr_dbd_mysql.la"
     test $apu_have_sqlite2 = 1 && dsos="$dsos dbd/apr_dbd_sqlite2.la"
     test $apu_have_sqlite3 = 1 && dsos="$dsos dbd/apr_dbd_sqlite3.la"
     test $apu_have_freetds = 1 && dsos="$dsos dbd/apr_dbd_freetds.la"
     test $apu_has_ldap = 1 && dsos="$dsos ldap/apr_ldap.la"

     APU_MODULES="$APU_MODULES $dsos"
  fi
])
