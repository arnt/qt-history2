REQUIRES = !qt_one_lib
TARGET		= q3compat
include(qbase.pri)
QCONFIG = core gui network

DEFINES += QT_BUILD_COMPAT_LIB

include($$COMPAT_CPP/qt_compat.pri)

mac:LIBS += -framework Carbon
