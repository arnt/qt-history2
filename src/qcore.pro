REQUIRES = !qt_one_lib
TARGET		= qcore
QCONFIG         =

DEFINES += QT_BUILD_CORE_LIB 

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = $$KERNEL_CPP/qt_pch.h

include(qbase.pri)
include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)
include($$KERNEL_CPP/qt_core.pri)
include($$THREAD_CPP/qt_thread.pri)
include($$TOOLS_CPP/qt_tools.pri)
include($$CODECS_CPP/qt_codecs.pri)

mac:LIBS += -framework CoreServices -framework CoreFoundation

