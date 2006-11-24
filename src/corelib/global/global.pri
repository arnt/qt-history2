# Qt kernel library base module

HEADERS +=  \
	global/qglobal.h \
	global/qnamespace.h \
        global/qendian.h \
        global/qnumeric_p.h \
        global/qnumeric.h

SOURCES += \
	global/qglobal.cpp \
        global/qlibraryinfo.cpp \
        global/qnumeric.cpp

# qlibraryinfo.cpp includes qconfig.cpp
INCLUDEPATH += $$QT_BUILD_TREE/src/corelib/global

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = global/qt_pch.h
