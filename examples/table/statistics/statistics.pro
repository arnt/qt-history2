TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS	= statistics.h
SOURCES	= statistics.cpp main.cpp
TARGET		= statistics
DEPENDPATH	= ../../include
REQUIRES=table full-config

# MSVC.NET projects do not accept the quotation marks
win32-msvc.net:contains(TEMPLATE_PREFIX,vc):DEFINES += QT_SOURCE_TREE=$$QT_SOURCE_TREE
else:DEFINES += QT_SOURCE_TREE="\"$$QT_SOURCE_TREE\""
