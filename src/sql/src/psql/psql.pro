TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin

HEADERS		= qsql_psql.h 

SOURCES		= main.cpp \
		  qsql_psql.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/postgresql
win32:INCLUDEPATH += C:\HOME\DB\SRC\POSTGR~1.2\SRC\INTERF~1\LIBPQ 

unix:TMAKE_LFLAGS += -L/usr/local/pgsql/lib

unix:LIBS       += -lpq 
win32:LIBS	+= libpqdll.lib 

TARGET		= qsqlpsql
DESTDIR		= ../../../../plugins

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
