REQUIRES = !qt_one_lib
TARGET		= q3compat
include(qbase.pri)

DEFINES += QT_BUILD_COMPAT_LIB

QCONFIG = kernel gui
include($$COMPAT_CPP/qt_compat.pri)
