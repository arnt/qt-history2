REQUIRES = !qt_one_lib
TARGET		= qcore
QCONFIG         =

CONFIG += console
CONFIG -= opengl x11sm

DEFINES += QT_BUILD_CORE_LIB 

include(../qbase.pri)
include(arch/$$ARCH/arch.pri)
include(global/global.pri)
include(thread/thread.pri)
include(tools/tools.pri)
include(io/io.pri)
include(library/library.pri)
include(kernel/kernel.pri)
include(codecs/codecs.pri)

mac:LIBS += -framework CoreServices -framework CoreFoundation -framework ApplicationServices

win32:DEFINES-=QT_NO_CAST_TO_ASCII
