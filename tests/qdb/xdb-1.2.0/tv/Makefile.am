## Process this file with automake to produce Makefile.in
##
#	This file is part of the XBase libraries
#    Copyright (C) 1998,1999 Vitaly Fedrushkov <fedrushkov@acm.org>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public
#    License along with this library; see the file COPYING.LIB.  If not,
#    write to the Free Software Foundation, Inc., or see
#    http://www.gnu.org/.

INCLUDES = -I$(topdir) $(all_includes)

noinst_PROGRAMS = @TVPROGS@
EXTRA_PROGRAMS = tvxbase

tvxbase_SOURCES = tvxbase.cpp tvbrowse.cpp tvbrowse.h
tvxbase_LDADD = -L$(topdir)/xbase/.libs -lxbase -ltvision -lgpm

MAINTAINERCLEANFILES = Makefile.in
