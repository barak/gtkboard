%define name	gtkboard
%define ver	0.9.3
%define rel	1

Summary: Board games suite
Name: %name
Version: %ver
Release: %rel
Copyright: GPL
Group: Games
URL: http://gtkboard.sourceforge.net/
Source: %{name}-%{version}.tar.gz
BuildRoot: /tmp/%{name}-root
Prefix: /usr/local
Docdir: /usr/local/doc

%description
gtkboard is a board games architecture and implementation.

%prep
%setup -q

%build
%configure
make

%install
#if [ -d $RPM_BUILD_ROOT ]; then rm -r $RPM_BUILD_ROOT ; fi
#mkdir -p $RPM_BUILD_ROOT/usr/local/bin
#install -s -m 755 -o 0 -g 0 src/gtkboard $RPM_BUILD_ROOT/usr/local/bin/gtkboard
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/gtkboard
%doc COPYING README AUTHORS ChangeLog doc/index.html doc/devel/index.html

