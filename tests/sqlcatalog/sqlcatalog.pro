TEMPLATE    	= app
CONFIG+= qt console warn_on debug
win32:DEFINES  	+= QT_DLL
SOURCES		+= sqlcatalog.cpp

win32:OBJECTS_DIR = obj
unix:OBJECTS_DIR  = .obj
#unix:LIBS	+= -lpthread

TARGET      	= sqlcatalog
