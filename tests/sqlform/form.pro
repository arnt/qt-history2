TEMPLATE	= app
CONFIG+= qt warn_on release
HEADERS		= form.h
SOURCES		= form.cpp \
		  main.cpp
INTERFACES	= formbase.ui
TARGET          = form
unix:LIBS      += -lpthread
