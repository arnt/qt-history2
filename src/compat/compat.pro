# Qt compat library

REQUIRES = !qt_one_lib
TARGET		= q3compat
QCONFIG         = core gui network sql

DEFINES += QT_BUILD_COMPAT_LIB 

include(../qbase.pri)

PRECOMPILED_HEADER = ../gui/base/qt_gui_pch.h

include(tools/tools.pri)
include(other/other.pri)
include(containers/containers.pri)

mac:LIBS += -framework Carbon

