#	This file is part of the XBase libraries
#    Copyright (C) 1998 Denis Pershin (dyp@inetlab.com)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License 
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(topdir)

lib_LTLIBRARIES = libxdb.la

pkginclude_HEADERS = dbf.h exp.h html.h ndx.h options.h retcodes.h xbase.h \
	xdate.h xstack.h xtypes.h xbexcept.h xbstring.h index.h ntx.h   \
        xbconfig.h

install-data-hook:
	(cd $(includedir); rm -f xbase.h; ln -s xbase/xbase.h xbase.h)

libxdb_la_SOURCES = dbf.cpp exp.cpp expfunc.cpp expproc.cpp fields.cpp \
	html.cpp lock.cpp memo.cpp ndx.cpp stack.cpp xbase.cpp xdate.cpp \
	xbexcept.cpp xbstring.cpp index.cpp ntx.cpp

#libxdb_la_LDFLAGS = -version-info 0:0:0
libxdb_la_LDFLAGS = -release 1.2.0
libxdb_la_LIBADD = 

MAINTAINERCLEANFILES = Makefile.in stamp-h.in xbconfig.h.in
