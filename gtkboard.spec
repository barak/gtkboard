%define name	gtkboard
%define ver	0.11pre0
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
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/gtkboard
%doc COPYING README AUTHORS ChangeLog doc/

