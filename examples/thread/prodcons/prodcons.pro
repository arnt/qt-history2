TEMPLATE	= app
TARGET		= prodcons

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

SOURCES		= prodcons.cpp
CLEAN_FILES	= prodcons.out
QT	+= compat
