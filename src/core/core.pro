TARGET		= QtCore
QPRO_PWD        = $$PWD
QT         =

DEFINES += QT_BUILD_CORE_LIB

!exceptions::DEFINES += QT_NO_EXCEPTIONS

include(../qbase.pri)
include(arch/$$ARCH/arch.pri)
include(global/global.pri)
include(thread/thread.pri)
include(tools/tools.pri)
include(io/io.pri)
include(library/library.pri)
include(kernel/kernel.pri)
include(codecs/codecs.pri)

mac:LIBS += -framework ApplicationServices

win32:DEFINES-=QT_NO_CAST_TO_ASCII

QMAKE_LIBS += $$QMAKE_LIBS_CORE
