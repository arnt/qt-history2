TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= domtree.h \
		  xmlfileitem.h \
		  nodeedit.h \
		  controlcentral.h
SOURCES		= main.cpp \
		  domtree.cpp \
		  xmlfileitem.cpp \
		  nodeedit.cpp \
		  controlcentral.cpp
INTERFACES	=
TARGET		= domtest
REQUIRES	= xml
