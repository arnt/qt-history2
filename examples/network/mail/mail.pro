TEMPLATE	= app
TARGET		= mail

QT         += network compat
CONFIG		+= qt uic3 warn_on release

HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
