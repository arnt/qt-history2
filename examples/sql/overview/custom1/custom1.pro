TEMPLATE    =	app
CONFIG+=	qt warn_on release
HEADERS	    =	main.h
SOURCES	    =	main.cpp ../connection.cpp
QTDIR_build:REQUIRES=full-config
