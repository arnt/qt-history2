REQUIRES = !qt_one_lib
TARGET		= q3compat
include(qbase.pri)

QCONFIG = kernel gui
include($$COMPAT_CPP/qt_compat.pri)
