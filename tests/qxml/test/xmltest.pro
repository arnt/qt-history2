TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= controlcentral.h \
		  itemtextview.h \
		  xmlfileitem.h \
		  xmlparser.h
SOURCES		= controlcentral.cpp \
		  itemtextview.cpp \
		  xmlfileitem.cpp \
		  xmlparser.cpp \
		  xmltest.cpp
INTERFACES	= 
TARGET		= xmltest
