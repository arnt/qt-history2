TEMPLATE	= app
LANGUAGE	= C++

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES 	= full-config nocrosscompiler

SOURCES		+= main.cpp
FORMS		= mainform.ui \
		  dialogform.ui \
		  extension.ui
DBFILE		= extension.db
