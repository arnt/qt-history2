TEMPLATE	= app
CONFIG+= qt warn_on release
win32:CONFIG+= console
HEADERS		= 
SOURCES		= main.cpp
INTERFACES	= 
TARGET          = blob
QTDIR_build:REQUIRES=full-config
