TARGET	   = QtSql
QPRO_PWD   = $$PWD
QT         = core
DEFINES += QT_BUILD_SQL_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x62000000

include(../qbase.pri)

DEFINES += QT_NO_CAST_FROM_ASCII
PRECOMPILED_HEADER = ../corelib/global/qt_pch.h
SQL_P = sql

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(models/models.pri)

