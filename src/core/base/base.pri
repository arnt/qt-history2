# Qt kernel library base module

HEADERS +=  \
	base/qglobal.h \
	base/qnamespace.h 

SOURCES += base/qglobal.cpp

# qconfig.cpp
exists($$QT_BUILD_TREE/src/core/base/qconfig.cpp) {
    SOURCES += $$QT_BUILD_TREE/src/core/base/qconfig.cpp
}

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = base/qt_pch.h
