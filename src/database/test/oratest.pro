TEMPLATE    	= app
CONFIG      	= qt console warn_on debug
DEFINES	    	+= QT_DLL \
	       	QT_SQL_SUPPORT
HEADERS     	= ../qsql.h \
		../qsqldatabase.h \
		../qsqldriver.h \
		../qsqlresult.h \
		../qsqlresultinfo.h \
		../qsqlerror.h \
		../qsqldriverplugin.h

SOURCES     	= ../qsql.cpp \
		../qsqldatabase.cpp \
		../qsqldriver.cpp \
		../qsqlresult.cpp \
		../qsqlresultinfo.cpp \
		../qsqlerror.cpp \
	       	../qsqldriverplugin.cpp \
	      	oratest.cpp

OBJECTS_DIR  	= .obj

INCLUDEPATH 	+= ../

TARGET      	= oratest