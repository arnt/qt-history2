TARGET	   = Qt3Support
QPRO_PWD   = $$PWD
QT         = core gui network sql
DEFINES   += QT_BUILD_COMPAT_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x60000000

include(../qbase.pri)
DEFINES -= QT_ASCII_CAST_WARNINGS

!win32-icc:PRECOMPILED_HEADER = other/qt_compat_pch.h

include(tools/tools.pri)
include(sql/sql.pri)
include(other/other.pri)
include(itemviews/itemviews.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
include(text/text.pri)
include(canvas/canvas.pri)
include(network/network.pri)
include(painting/painting.pri)

mac:LIBS += -framework Carbon

QMAKE_LIBS += $$QMAKE_LIBS_COMPAT $$QMAKE_LIBS_NETWORK
DEFINES -= QT3_SUPPORT_WARNINGS
DEFINES += QT3_SUPPORT
MOCDIR = .moc
