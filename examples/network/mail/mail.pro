TEMPLATE	= app
TARGET		= mail

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, full-config)"

HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
