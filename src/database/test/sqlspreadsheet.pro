TEMPLATE    = app
CONFIG      = qt warn_on debug
win32:DEFINES += QT_DLL \
		 QT_SQL_OCI_SUPPORT
DEFINES	    += QT_SQL_SUPPORT

HEADERS     = ../qsql.h \
		../qsqldatabase.h \
		../qsqldriver.h \
		../qsqlresult.h \
		../qsqlresultinfo.h \
		../qsqlerror.h \
		../qsqldriverplugin.h \
		spreadsheet.h \
		sqlspreadsheet.h \

SOURCES     = ../qsql.cpp \
		../qsqldatabase.cpp \
		../qsqldriver.cpp \
		../qsqlresult.cpp \
		../qsqlresultinfo.cpp \
		../qsqlerror.cpp \
	       ../qsqldriverplugin.cpp \
	       spreadsheet.cpp \
	       sqlspreadsheet.cpp \
	       spreadmain.cpp

INTERFACES	= spreadsheetwindow.ui

OBJECTS_DIR  = .obj

INCLUDEPATH	+= ../
unix:LIBS       += -ldl

TARGET      = spreadsheet