TEMPLATE	= app
TARGET		= semaphores

CONFIG		+= qt warn_on release thread

QTDIR_build:REQUIRES        = thread "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
