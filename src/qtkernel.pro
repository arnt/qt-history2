REQUIRES = !qt_one_lib
TARGET		= qtkernel
QCONFIG         =

DEFINES += QT_KERNEL_LIB

include(qbase.pri)
include($$KERNEL_CPP/qt_kernel.pri)
include($$THREAD_CPP/qt_thread.pri)
include($$TOOLS_CPP/qt_tools.pri)
include($$CODECS_CPP/qt_codecs.pri)
# include($$KERNEL_CPP/qt_gfx.pri)

mac:LIBS += -framework CoreServices -framework CoreFoundation

#qcompat, once the 4.0 removing of the compat stuff is complete, get this out of here.. (qt.pro too)
message("Move compat/* files into libqt3compat")
include($$COMPAT_CPP/qt_compat.pri)
