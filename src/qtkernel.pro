REQUIRES = !qt_one_lib
TARGET		= qtkernel
QCONFIG         =

include(qbase.pri)
include($$KERNEL_CPP/qt_kernel.pri)
include($$THREAD_CPP/qt_thread.pri)
include($$TOOLS_CPP/qt_tools.pri)
include($$CODECS_CPP/qt_codecs.pri)
include($$KERNEL_CPP/qt_gfx.pri)

#qcompat, once the 4.0 removing of the compat stuff is complete, get this out of here.. (qt.pro too)
message("Move compat/* files into libqt3compat")
include($$COMPAT_CPP/qt_compat.pri)
