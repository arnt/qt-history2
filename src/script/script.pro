TARGET     = QtScript
QPRO_PWD   = $$PWD
QT         = core
DEFINES   += QT_BUILD_SCRIPT_LIB
#win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000       ### FIXME

include(../qbase.pri)
include(script.pri)
