# Cherokee
# Copyright (C) 2001-2007 Alvaro Lopez Ortega

Name:           cherokee
Version:        0.9.2
Release:        1
License:        GPL
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Docdir:         %{_datadir}/doc
Autoreqprov:    on
Source:         %{name}-%{version}.tar.gz
Summary:        Flexible and Fast web server
Group:          Productivity/Networking/Web/Servers
URL:            http://www.cherokee-project.com

%description
Cherokee is a very fast, flexible and easy to configure Web Server.
It supports the widespread technologies nowadays: FastCGI, SCGI, PHP,
CGI, TLS and SSL encrypted connections, Virtual hosts, Authentication,
on the fly encoding, Apache compatible log files, and much more.

%prep
%setup -q
gzip -dc %SOURCE | tar xvf -

%build
CFLAGS="$RPM_OPT_FLAGS"                 \
./configure --prefix=%{_prefix}         \
            --sysconfdir=%{_sysconfdir} \
            --mandir=%{_mandir}         \
            --sbindir=%{_sbindir}       \
		  --with-wwwroot=/home/httpd/ \
            --enable-pthreads
make CFLAGS="-O0 -g3"

%install
make install DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/etc/cherokee/sites-enabled/default

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
ln -s %{_sysconfdir}/cherokee/sites-available/default \
      %{_sysconfdir}/cherokee/sites-enabled/default
mkdir -p /home/httpd

%postun
/sbin/ldconfig

%files
%{_mandir}/*
%{_bindir}/*
%{_libdir}/*
%{_datadir}/*
%{_sbindir}/*
%{_includedir}/*
%{_sysconfdir}/*
/home/httpd/*

