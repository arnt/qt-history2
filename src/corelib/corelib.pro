TARGET	   = QtCore
QPRO_PWD   = $$PWD
QT         =
DEFINES   += QT_BUILD_CORE_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x67000000

include(../qbase.pri)
include(arch/arch.pri)
include(global/global.pri)
include(thread/thread.pri)
include(tools/tools.pri)
include(io/io.pri)
include(plugin/plugin.pri)
include(kernel/kernel.pri)
include(codecs/codecs.pri)

mac:LIBS += -framework ApplicationServices

mac:lib_bundle:DEFINES += QT_NO_DEBUG_PLUGIN_CHECK
win32:DEFINES-=QT_NO_CAST_TO_ASCII

QMAKE_LIBS += $$QMAKE_LIBS_CORE

contains(DEFINES,QT_EVAL):include(eval.pri)
