GUID 		= {13d3fac4-2b1a-440b-972c-c1fe9b300c16}
TEMPLATE	= app
LANGUAGE	= C++

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES 	= full-config nocrosscompiler

SOURCES		+= main.cpp
FORMS		= mainform.ui \
		  dialogform.ui \
		  extension.ui
DBFILE		= extension.db
