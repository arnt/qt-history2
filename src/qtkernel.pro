REQUIRES = !qt_one_lib
TARGET		= qtkernel
QCONFIG         =

DEFINES += QT_BUILD_KERNEL_LIB 

include(qbase.pri)
include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)
include($$KERNEL_CPP/qt_kernel.pri)
include($$THREAD_CPP/qt_thread.pri)
include($$TOOLS_CPP/qt_tools.pri)
include($$CODECS_CPP/qt_codecs.pri)

mac:LIBS += -framework CoreServices -framework CoreFoundation

