exp_exec_prefix = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed
rel_exec_prefix =
exp_bindir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/bin
rel_bindir = bin
exp_sbindir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/bin
rel_sbindir = bin
exp_libdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/lib
rel_libdir = lib
exp_libexecdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/modules
rel_libexecdir = modules
exp_mandir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/man
rel_mandir = man
exp_sysconfdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/conf
rel_sysconfdir = conf
exp_datadir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed
rel_datadir =
exp_installbuilddir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/build
rel_installbuilddir = build
exp_errordir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/error
rel_errordir = error
exp_iconsdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/icons
rel_iconsdir = icons
exp_htdocsdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/htdocs
rel_htdocsdir = htdocs
exp_manualdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/manual
rel_manualdir = manual
exp_cgidir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/cgi-bin
rel_cgidir = cgi-bin
exp_includedir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/include
rel_includedir = include
exp_localstatedir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed
rel_localstatedir =
exp_runtimedir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/logs
rel_runtimedir = logs
exp_logfiledir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/logs
rel_logfiledir = logs
exp_proxycachedir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/proxy
rel_proxycachedir = proxy
SHLTCFLAGS = -prefer-pic
LTCFLAGS = -prefer-non-pic -static
MPM_NAME = prefork
MPM_SUBDIR_NAME = prefork
htpasswd_LTFLAGS =
htdigest_LTFLAGS =
rotatelogs_LTFLAGS =
logresolve_LTFLAGS =
htdbm_LTFLAGS =
ab_LTFLAGS =
checkgid_LTFLAGS =
APACHECTL_ULIMIT = ulimit -S -n `ulimit -H -n`
progname = httpd
MPM_LIB = server/mpm/prefork/libprefork.la
OS = unix
OS_DIR = unix
BUILTIN_LIBS = modules/aaa/mod_access.la modules/aaa/mod_auth.la modules/filters/mod_include.la modules/loggers/mod_log_config.la modules/metadata/mod_env.la modules/metadata/mod_setenvif.la modules/http/mod_http.la modules/http/mod_mime.la modules/generators/mod_status.la modules/generators/mod_autoindex.la modules/generators/mod_asis.la modules/generators/mod_cgi.la modules/mappers/mod_negotiation.la modules/mappers/mod_dir.la modules/mappers/mod_imap.la modules/mappers/mod_actions.la modules/mappers/mod_userdir.la modules/mappers/mod_alias.la modules/mappers/mod_so.la
SHLIBPATH_VAR = LD_LIBRARY_PATH
OS_SPECIFIC_VARS =
PRE_SHARED_CMDS = echo ""
POST_SHARED_CMDS = echo ""
shared_build =
AP_LIBS = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/pcre/libpcre.la /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/libaprutil-0.la /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/xml/expat/lib/libexpat.la /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr/libapr-0.la -lrt -lm -lcrypt -lnsl -ldl
AP_BUILD_SRCLIB_DIRS = apr apr-util
AP_CLEAN_SRCLIB_DIRS = apr-util apr
abs_srcdir = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48
bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/bin
cgidir = ${datadir}/cgi-bin
logfiledir = ${localstatedir}/logs
exec_prefix = ${prefix}
datadir = ${prefix}
localstatedir = ${prefix}
mandir = ${prefix}/man
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/modules
htdocsdir = ${datadir}/htdocs
manualdir = ${datadir}/manual
includedir = ${prefix}/include
errordir = ${datadir}/error
iconsdir = ${datadir}/icons
sysconfdir = ${prefix}/conf
installbuilddir = ${datadir}/build
runtimedir = ${localstatedir}/logs
proxycachedir = ${localstatedir}/proxy
other_targets =
progname = httpd
prefix = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed
AWK = mawk
CC = gcc
CPP = gcc -E
CXX =
CPPFLAGS =
CFLAGS =
CXXFLAGS =
LTFLAGS = --silent
LDFLAGS =
LT_LDFLAGS = -export-dynamic
SH_LDFLAGS =
HTTPD_LDFLAGS =
UTIL_LDFLAGS =
LIBS =
DEFS =
INCLUDES =
NOTEST_CPPFLAGS = -DAP_HAVE_DESIGNATED_INITIALIZER
NOTEST_CFLAGS =
NOTEST_CXXFLAGS =
NOTEST_LDFLAGS =
NOTEST_LIBS =
EXTRA_CPPFLAGS = -D_REENTRANT -D_GNU_SOURCE
EXTRA_CFLAGS = -g -O2 -pthread
EXTRA_CXXFLAGS =
EXTRA_LDFLAGS = -L/home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/xml/expat/lib
EXTRA_LIBS =
EXTRA_INCLUDES = -I/home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr/include -I/home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/include -I/home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/xml/expat/lib -I. -I$(top_srcdir)/os/$(OS_DIR) -I$(top_srcdir)/server/mpm/$(MPM_SUBDIR_NAME) -I$(top_srcdir)/modules/http -I$(top_srcdir)/modules/filters -I$(top_srcdir)/modules/proxy -I$(top_srcdir)/include -I$(top_srcdir)/modules/dav/main
LIBTOOL = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr/libtool $(LTFLAGS)
SHELL = /bin/bash
MODULE_DIRS = aaa filters loggers metadata http generators mappers
MODULE_CLEANDIRS = arch/win32 cache echo experimental proxy ssl test dav/main dav/fs
PORT = 80
nonssl_listen_stmt_1 =
nonssl_listen_stmt_2 = Listen @@Port@@
CORE_IMPLIB_FILE =
CORE_IMPLIB =
SH_LIBS =
SH_LIBTOOL = $(LIBTOOL)
MK_IMPLIB =
INSTALL_PROG_FLAGS =
DSO_MODULES =
APR_BINDIR = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/bin
APR_INCLUDEDIR = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr/include
APU_BINDIR = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/installed/bin
APU_INCLUDEDIR = /home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr-util/include
