QTDIR_build:REQUIRES        = network full-config
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
TARGET		= mail
