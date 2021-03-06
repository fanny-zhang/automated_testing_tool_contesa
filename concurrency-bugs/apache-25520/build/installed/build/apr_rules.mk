# ====================================================================
# The Apache Software License, Version 1.1
#
# Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# 3. The end-user documentation included with the redistribution,
#    if any, must include the following acknowledgment:
#       "This product includes software developed by the
#        Apache Software Foundation (http://www.apache.org/)."
#    Alternately, this acknowledgment may appear in the software itself,
#    if and wherever such third-party acknowledgments normally appear.
#
# 4. The names "Apache" and "Apache Software Foundation" must
#    not be used to endorse or promote products derived from this
#    software without prior written permission. For written
#    permission, please contact apache@apache.org.
#
# 5. Products derived from this software may not be called "Apache",
#    nor may "Apache" appear in their name, without prior written
#    permission of the Apache Software Foundation.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
# ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# ====================================================================
#
# This software consists of voluntary contributions made by many
# individuals on behalf of the Apache Software Foundation.  For more
# information on the Apache Software Foundation, please see
# <http://www.apache.org/>.
#

#
# rules.mk: standard rules for APR
#



#
# Configuration variables
#
apr_builddir=/home/tyu/Documents/concurrency-bugs/apache-25520/build/srclib/apr
apr_builders=/home/tyu/Documents/concurrency-bugs/apache-25520/httpd-2.0.48/srclib/apr/build

# Some layouts require knowing what version we are at.
APR_MAJOR_VERSION=0
APR_DOTTED_VERSION=0.9.5

CC=/home/tyu/Documents/llvm/Release+Asserts/bin/clang
RM=rm
AWK=mawk
SHELL=/bin/bash
LIBTOOL=$(SHELL) $(apr_builddir)/libtool

# compilation and linking flags that are supposed to be set only by the user.
# configure adds to them for tests, but we restore them at the end.
#
CFLAGS=-emit-llvm -flto -g
CPPFLAGS=
LDFLAGS=
LIBS=
DEFS=-DHAVE_CONFIG_H

# anything added to the standard flags by configure is moved to EXTRA_*
# at the end of the process.
#
EXTRA_CFLAGS= -pthread
EXTRA_CPPFLAGS=-D_REENTRANT -D_GNU_SOURCE
EXTRA_LDFLAGS=
EXTRA_LIBS=-lrt -lm -lcrypt -lnsl 
EXTRA_INCLUDES=

# NOTEST_* are flags and libraries that can be added by the user without
# causing them to be used in configure tests (necessary for things like
# -Werror and other strict warnings that maintainers like to use).
#
NOTEST_CFLAGS=
NOTEST_CPPFLAGS=
NOTEST_LDFLAGS=
NOTEST_LIBS=

# Finally, combine all of the flags together in the proper order so that
# the user-defined flags can always override the configure ones, if needed.
# Note that includes are listed after the flags because -I options have
# left-to-right precedence and CPPFLAGS may include user-defined overrides.
#
ALL_CFLAGS   = $(EXTRA_CFLAGS) $(NOTEST_CFLAGS) $(CFLAGS)
ALL_CPPFLAGS = $(DEFS) $(EXTRA_CPPFLAGS) $(NOTEST_CPPFLAGS) $(CPPFLAGS)
ALL_LDFLAGS  = $(EXTRA_LDFLAGS) $(NOTEST_LDFLAGS) $(LDFLAGS)
ALL_LIBS     = $(LIBS) $(NOTEST_LIBS) $(EXTRA_LIBS)
ALL_INCLUDES = $(INCLUDES) $(EXTRA_INCLUDES)

LTFLAGS      = --silent
LT_LDFLAGS   = 

#
# Basic macro setup
#
COMPILE      = $(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(ALL_INCLUDES)
LT_COMPILE   = $(LIBTOOL) $(LTFLAGS) --mode=compile $(COMPILE) -c $< && touch $@

LINK         = $(LIBTOOL) $(LTFLAGS) --mode=link $(LT_LDFLAGS) $(COMPILE) -version-info 9:5:9 $(ALL_LDFLAGS) -o $@

MKEXPORT     = $(AWK) -f $(apr_builders)/make_export.awk
MKDEP        = $(CC) -MM

#
# Standard build rules
#
all: all-recursive
depend: depend-recursive
clean: clean-recursive
distclean: distclean-recursive
extraclean: extraclean-recursive

install: all-recursive


all-recursive depend-recursive:
	@otarget=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; \
	for i in $$list; do \
	    if test -d "$$i"; then \
		target="$$otarget"; \
		echo "Making $$target in $$i"; \
		if test "$$i" = "."; then \
		    made_local=yes; \
		    target="local-$$target"; \
		fi; \
		(cd $$i && $(MAKE) $$target) || exit 1; \
	    fi; \
	done; \
        if test "$$otarget" = "all" && test -z "$(TARGETS)"; then \
	    made_local=yes; \
	fi; \
	if test "$$made_local" != "yes"; then \
	    $(MAKE) "local-$$otarget" || exit 1; \
	fi

clean-recursive distclean-recursive extraclean-recursive:
	@otarget=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS) $(CLEAN_SUBDIRS)'; \
	for i in $$list; do \
	    if test -d "$$i"; then \
		target="$$otarget"; \
		echo "Making $$target in $$i"; \
		if test "$$i" = "."; then \
		    made_local=yes; \
		    target="local-$$target"; \
		fi; \
		(cd $$i && $(MAKE) $$target); \
	    fi; \
	done; \
        if test "$$otarget" = "all" && test -z "$(TARGETS)"; then \
	    made_local=yes; \
	fi; \
	if test "$$made_local" != "yes"; then \
	    $(MAKE) "local-$$otarget"; \
	fi

# autoconf 2.5x is creating a 'autom4te.cache' directory
# In case someone ran autoconf by hand, get rid of that directory
# aswell.
local-clean: x-local-clean
	$(RM) -f *.o *.lo *.a *.la *.so *.obj $(CLEAN_TARGETS) $(PROGRAMS)
	$(RM) -rf .libs autom4te.cache

local-distclean: local-clean x-local-distclean
	$(RM) -f Makefile $(DISTCLEAN_TARGETS)

local-extraclean: local-distclean x-local-extraclean
	@if test -n "$(EXTRACLEAN_TARGETS)"; then \
	    echo $(RM) -f $(EXTRACLEAN_TARGETS) ; \
	    $(RM) -f $(EXTRACLEAN_TARGETS) ; \
	fi

local-all: $(TARGETS)

local-depend: x-local-depend
	@if test -n "`ls $(srcdir)/*.c 2> /dev/null`"; then \
		$(RM) -f .deps; \
		list='$(srcdir)/*.c'; \
		for i in $$list; do \
			$(MKDEP) $(ALL_CPPFLAGS) $(ALL_INCLUDES) $$i | sed 's/\.o:/.lo:/' >> .deps; \
		done; \
	fi

# to be filled in by the actual Makefile
x-local-depend x-local-clean x-local-distclean x-local-extraclean:

#
# Implicit rules for creating outputs from input files
#
.SUFFIXES:
.SUFFIXES: .c .lo .o

.c.o:
	$(COMPILE) -c $<

.c.lo:
	$(LT_COMPILE)

.PHONY: all all-recursive local-all install \
	depend depend-recursive local-depend x-local-depend \
	clean clean-recursive local-clean x-local-clean \
	distclean distclean-recursive local-distclean x-local-distclean \
	extraclean extraclean-recursive local-extraclean x-local-extraclean
