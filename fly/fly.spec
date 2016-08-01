Name: fly
Summary: fly - a C++ utility library
URL: https://github.com/trflynn89/libfly
Group: Development/Libraries
Version: %{version}
Release: 1
License: MIT

%package devel
Summary: fly - a C++ utility library
Group: Development/Libraries
Requires: gcc-c++ >= 5.0, %{name} = %{version}-%{release}

%description
fly, a C++ utility library for Linux and Windows.

%description devel
Development files for using fly for development.

%prep
pushd $RPM_BUILD_ROOT > /dev/null

for f in `find .%{_libdir} -type f -name "*\.so\.*\.*\.*"` ; do
    src=${f:1}
    dst=${src%.*}
    ext=${src##*.}

    while [ "$ext" != "so" ] ; do
        ln -sf $src .$dst

        src=$dst
        dst=${dst%.*}
        ext=${src##*.}
    done
done

popd > /dev/null

%post -p /sbin/ldconfig
%post devel -p /sbin/ldconfig

%postun -p /sbin/ldconfig
%postun devel -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc
%{_libdir}/*.so.*

%files devel
%defattr(-, root, root)
%doc
%{_libdir}/*.so
%{_includedir}/*
%{_usrsrc}/*
