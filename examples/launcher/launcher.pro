TEMPLATE	= app
TARGET		= launcher

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = embedded "contains(QT_CONFIG, large-config)"

HEADERS		=
SOURCES		= launcher.cpp
