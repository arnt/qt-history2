# Qt compat library

REQUIRES = !qt_one_lib
TARGET		= Qt3Compat
QT         = core gui network sql

DEFINES += QT_BUILD_COMPAT_LIB 

include(../qbase.pri)

PRECOMPILED_HEADER = ../gui/kernel/qt_gui_pch.h

include(tools/tools.pri)
include(sql/sql.pri)
include(other/other.pri)
include(itemviews/itemviews.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)

mac:LIBS += -framework Carbon

QMAKE_LIBS += $$QMAKE_LIBS_COMPAT
