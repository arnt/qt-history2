TEMPLATE	= app
CONFIG+= qt warn_on debug 
HEADERS		= search.h
SOURCES		= search.cpp \
		  main.cpp
INTERFACES	= searchbase.ui
TARGET          = search 
unix:LIBS      += -lpthread
