#    Xbase project source code
#
#    This file contains the qmake project for building the XBase SQL library
#
#    Copyright (C) 2000 Dave Berton (db@trolltech.com)
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    Contact:
#
#      Mail:
#
#        Technology Associates, Inc.
#        XBase Project
#        1455 Deming Way #11
#        Sparks, NV 89434
#        USA
#
#      Email:
#
#        xbase@techass.com
#
#      See our website at:
#
#        xdb.sourceforge.net
TEMPLATE	= lib
CONFIG 		= xbase
include(../include/shared.pri)
SOURCES	        += ../src/sqlinterpreter.cpp \
		../src/filedriver_xbase.cpp \
		../src/op.cpp \
		../src/parser.cpp \
		../src/qdb.cpp 
LIBS		+= -lxdb
TARGET          += qdb
VERSION		+= 1.0.0
DESTDIR		+= ../lib

# install
target.path=$$QDB_INSTALL_LIBPATH
isEmpty(target.path):target.path=/usr/local/lib
INSTALLS += target

#headers
headers.path=$$QDB_INSTALL_HEADERPATH
isEmpty(headers.path):headers.path=/usr/local/include/qdb
headers.files = ../include/*.h
INSTALLS += headers
