TEMPLATE	= app
CONFIG		= qt warn_on debug
INCLUDEPATH	= ../xdb-1.2.0/
LIBS		= -lxdb
SOURCES	        = main.cpp sqlinterpreter.cpp environment.cpp filedriver_xbase.cpp
HEADERS		= sqlinterpreter.h environment.h
TARGET          = sqlinterpreter
