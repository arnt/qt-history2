QTDIR_build:REQUIRES        = thread
TEMPLATE	= app
CONFIG		+= qt warn_on release thread
HEADERS		= 
SOURCES		= main.cpp
INTERFACES	= 
TARGET		= semaphores
QTDIR_build:REQUIRES=full-config
