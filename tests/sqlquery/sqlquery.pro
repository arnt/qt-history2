TEMPLATE    	= app
CONFIG      	= qt warn_on debug
win32:DEFINES	+= QT_DLL

INTERFACES 	+= databasechooserdialog.ui \
		sqlquerywindow.ui

HEADERS     	= browse.h \
		sqlquery.h

SOURCES     	= sqlquery.cpp \

unix:OBJECTS_DIR		= .obj
win32:OBJECTS_DIR		= obj

#unix:LIBS	+= -lpthread

TARGET      	= sqlquery