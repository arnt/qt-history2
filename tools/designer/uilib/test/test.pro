TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= test
unix:LIBS		+= -lqui -L$$QT_BUILD_TREE/lib
win32:LIBS	+= $$QT_BUILD_TREE/lib/qui.lib
