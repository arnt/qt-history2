TEMPLATE = lib
TARGET	 = qsqlite

CONFIG	+= qt plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/sqldrivers

HEADERS		= ../../../sql/drivers/sqlite/qsql_sqlite.h
SOURCES		= smain.cpp \
		  ../../../sql/drivers/sqlite/qsql_sqlite.cpp

unix {
	OBJECTS_DIR = .obj
}

win32 {
	OBJECTS_DIR = obj
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libsqlite.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibsqlite.dll
#	}
}

!contains( LIBS, .*sqlite.* ) {
    INCLUDEPATH += ../../../3rdparty/sqlite

    HEADERS += ../../../3rdparty/sqlite/btree.h \
               ../../../3rdparty/sqlite/config.h \
               ../../../3rdparty/sqlite/hash.h \
               ../../../3rdparty/sqlite/opcodes.h \
               ../../../3rdparty/sqlite/os.h \
               ../../../3rdparty/sqlite/pager.h \
               ../../../3rdparty/sqlite/parse.h \
               ../../../3rdparty/sqlite/sqlite.h \
               ../../../3rdparty/sqlite/sqliteInt.h \
               ../../../3rdparty/sqlite/vdbe.h \
	       ../../../3rdparty/sqlite/vdbeInt.h

    SOURCES += ../../../3rdparty/sqlite/attach.c \
               ../../../3rdparty/sqlite/auth.c \
               ../../../3rdparty/sqlite/btree.c \
               ../../../3rdparty/sqlite/btree_rb.c \
               ../../../3rdparty/sqlite/build.c \
               ../../../3rdparty/sqlite/copy.c \
	       ../../../3rdparty/sqlite/date.c \
               ../../../3rdparty/sqlite/delete.c \
               ../../../3rdparty/sqlite/expr.c \
               ../../../3rdparty/sqlite/func.c \
               ../../../3rdparty/sqlite/hash.c \
               ../../../3rdparty/sqlite/insert.c \
               ../../../3rdparty/sqlite/main.c \
               ../../../3rdparty/sqlite/opcodes.c \
               ../../../3rdparty/sqlite/os.c \
               ../../../3rdparty/sqlite/pager.c \
               ../../../3rdparty/sqlite/parse.c \
               ../../../3rdparty/sqlite/pragma.c \
               ../../../3rdparty/sqlite/printf.c \
               ../../../3rdparty/sqlite/random.c \
               ../../../3rdparty/sqlite/select.c \
               ../../../3rdparty/sqlite/shell.c \
               ../../../3rdparty/sqlite/table.c \
               ../../../3rdparty/sqlite/tokenize.c \
               ../../../3rdparty/sqlite/trigger.c \
               ../../../3rdparty/sqlite/update.c \
               ../../../3rdparty/sqlite/util.c \
               ../../../3rdparty/sqlite/vacuum.c \
               ../../../3rdparty/sqlite/vdbe.c \
	       ../../../3rdparty/sqlite/vdbeaux.c \
               ../../../3rdparty/sqlite/where.c
}

QTDIR_build:REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
