# Qt kernel library base module

HEADERS +=  \
	global/qglobal.h \
	global/qfixedpoint.h \
	global/qnamespace.h 

SOURCES += \
	global/qglobal.cpp \
	global/qfixedpoint.cpp

# qconfig.cpp
exists($$QT_BUILD_TREE/src/core/global/qconfig.cpp) {
    SOURCES += global/qconfig.cpp
}

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = global/qt_pch.h
