TARGET		= QtSql
QPRO_PWD        = $$PWD
QT = core

DEFINES += QT_BUILD_SQL_LIB

DEFINES += QT_NO_CAST_FROM_ASCII

PRECOMPILED_HEADER = ../core/global/qt_pch.h

include(../qbase.pri)

SQL_P =         sql

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(gui/gui.pri)

