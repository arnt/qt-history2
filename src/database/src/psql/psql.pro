TEMPLATE	= lib
CONFIG		= qt warn_on debug
win32:DEFINES 	+= QT_DLL
win32:CONFIG    += dll

HEADERS		= qsql_psql.h 

SOURCES		= main.cpp \
		  qsql_psql.cpp 

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:INCLUDEPATH += /usr/include/postgresql
win32:INCLUDEPATH += C:\HOME\DB\SRC\POSTGR~1.2\SRC\INTERF~1\LIBPQ 

unix:TMAKE_LFLAGS += -L/usr/local/pgsql/lib

unix:LIBS       += -lpq 
win32:LIBS	+= C:\HOME\DB\SRC\POSTGR~1.2\SRC\INTERF~1\LIBPQ\RELEASE\libpqdll.lib 

TARGET		= qsqlpsql
DESTDIR		= $(QTDIR)/lib
