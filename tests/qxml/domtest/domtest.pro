TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= domtree.h \
		  xmlfileitem.h \
		  controlcentral.h
SOURCES		= main.cpp \
		  domtree.cpp \
		  xmlfileitem.cpp \
		  controlcentral.cpp
INTERFACES	=
TARGET		= domtest
REQUIRES	= xml
