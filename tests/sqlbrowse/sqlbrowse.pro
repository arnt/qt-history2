TEMPLATE    	= app
CONFIG+= qt warn_on debug
win32:DEFINES	+= QT_DLL

INTERFACES 	+= sqlbrowsewindow.ui

HEADERS     	= resultwindow.h 

SOURCES     	= resultwindow.cpp \
		main.cpp

unix:OBJECTS_DIR		= .obj
win32:OBJECTS_DIR		= obj

#unix:LIBS	+= -lpthread

TARGET      	= sqlbrowse
