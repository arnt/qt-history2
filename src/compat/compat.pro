# Qt compat library

REQUIRES = !qt_one_lib
TARGET		= q3compat
QCONFIG         = core gui network sql

DEFINES += QT_BUILD_COMPAT_LIB 

include(../qbase.pri)
include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)

include(tools/tools.pri)
include(other/other.pri)

mac:LIBS += -framework Carbon

