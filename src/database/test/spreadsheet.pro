TEMPLATE    	= app
CONFIG      	= qt warn_on debug
win32:DEFINES 	+= QT_DLL 

HEADERS     	=  spreadsheet.h 

SOURCES     	=  spreadsheet.cpp \
		  main.cpp

INTERFACES	= simplespreadsheetwindow.ui

OBJECTS_DIR  	= .obj

LIBS		+= -ldl

TARGET      	= spreadsheet