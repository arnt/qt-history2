# Additional Qt project file for qtmain lib on Windows
TEMPLATE = lib
TARGET	 = qtmain
DESTDIR	 = $$QMAKE_LIBDIR_QT

CONFIG	+= staticlib warn_on release
CONFIG	-= qt

win32 {
	SOURCES		= qtmain_win.cpp
	CONFIG		+= png zlib
	CONFIG		-= jpeg
	INCLUDEPATH	+= tmp
}

!win32-*:!wince-*:error("${QMAKE_FILE} is intended only for Windows!")
