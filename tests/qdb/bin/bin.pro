#    This file contains the qmake project for building the LocalSQL 
#    command line program
#
#    Copyright (C) 2001 Trolltech AS
#
#    Contact:
#            Dave Berton (db@trolltech.com)
#            Jasmin Blanchette (jasmin@trolltech.com)
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
TEMPLATE	= app
CONFIG+= xbase
include(../include/shared.pri)
SOURCES	        += main.cpp
QMAKE_LIBDIR		+= ../lib
LIBS		+= -llsql
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = lsql

# install
target.path=$$QDB_INSTALL_BINPATH
isEmpty(target.path):target.path=/usr/local/bin
INSTALLS += target
