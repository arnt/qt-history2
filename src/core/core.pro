REQUIRES = !qt_one_lib
TARGET		= qcore
QCONFIG         =

CONFIG += console
CONFIG -= opengl x11sm

DEFINES += QT_BUILD_CORE_LIB 

include(../qbase.pri)
include(arch/$$ARCH/arch.pri)
include(base/base.pri)
include(thread/thread.pri)
include(tools/tools.pri)
include(io/io.pri)
include(library/library.pri)
include(codecs/codecs.pri)
include(object/object.pri)
include(other/other.pri)

mac:LIBS += -framework CoreServices -framework CoreFoundation

win32:DEFINES-=QT_NO_CAST_TO_ASCII
