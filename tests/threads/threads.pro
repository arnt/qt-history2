TEMPLATE	= app
CONFIG		= qt warn_on release console
HEADERS		= 
SOURCES		= threads.cpp
TARGET		= threads
DEFINES		+=QT_THREAD_SUPPORT
unix:LIBS	+= -lpthread
