TEMPLATE    	= app
CONFIG+= qt warn_on debug
win32:DEFINES	+= QT_DLL

INTERFACES 	+= masterchildwindow.ui

HEADERS     	= mainwindow.h 

SOURCES     	= mainwindow.cpp \
		main.cpp

unix:OBJECTS_DIR		= .obj
win32:OBJECTS_DIR		= obj

TARGET      	= sqlmasterchild
