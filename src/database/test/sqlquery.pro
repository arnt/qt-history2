TEMPLATE    	= app
CONFIG      	= qt warn_on debug
DEFINES 	+= QT_SQL_SUPPORT
win32:DEFINES	+= QT_DLL

INTERFACES 	+= databasechooserdialog.ui \
		sqlquerywindow.ui

HEADERS     	= ../qsql.h \
		../qsqldatabase.h \
		../qsqldriver.h \
		../qsqlresult.h \
		../qsqlresultinfo.h \
		../qsqlerror.h \
		../qsqldriverplugin.h \
		browse.h \
		sqlquery.h

SOURCES     	= ../qsql.cpp \
		../qsqldatabase.cpp \
		../qsqldriver.cpp \
		../qsqlresult.cpp \
		../qsqlresultinfo.cpp \
		../qsqlerror.cpp \
	       	../qsqldriverplugin.cpp \
               	sqlquery.cpp \

INCLUDEPATH 	+= ../

unix:OBJECTS_DIR		= .obj
win32:OBJECTS_DIR		= obj

unix:LIBS	+= -lpthread

TARGET      	= sqlquery