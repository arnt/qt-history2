# Qt kernel library base module

HEADERS +=  \
	global/qglobal.h \
	global/qfixedpoint.h \
	global/qnamespace.h 

SOURCES += \
	global/qglobal.cpp \
	global/qfixedpoint.cpp \
        global/qlibraryinfo.cpp 

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = global/qt_pch.h
