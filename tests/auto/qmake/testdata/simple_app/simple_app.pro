TEMPLATE		= app
CONFIG		+= qt warn_on
HEADERS		= test_file.h
SOURCES		= test_file.cpp \
		  	main.cpp
TARGET		= simple_app
DESTDIR		= ./

infile($(QTDIR)/.qmake.cache, CONFIG, debug):CONFIG += debug
infile($(QTDIR)/.qmake.cache, CONFIG, release):CONFIG += release

DEFINES += QT_USE_USING_NAMESPACE

