%define name	gtkboard
%define ver	0.9.2
%define rel	1

Summary: Board games suite
Name: %name
Version: %ver
Release: %rel
Copyright: GPL
Group: Games
Source: http://idunno.whatever.org/gtkboard-0.9.2.tar.gz
BuildRoot: /tmp/%{name}-root
Docdir: /usr/local/doc

%description
gtkboard is a board games architecture and implementation.

%prep
%setup -q

%build
%configure
make

%install
if [ -d $RPM_BUILD_ROOT ]; then rm -r $RPM_BUILD_ROOT ; fi
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
install -s -m 755 -o 0 -g 0 src/gtkboard $RPM_BUILD_ROOT/usr/local/bin/gtkboard

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/gtkboard
%doc COPYING README AUTHORS doc/

