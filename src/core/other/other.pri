# Qt core module

TOOLS_P= tools
HEADERS += \
	other/qcleanuphandler.h \
	other/qdebug.h \
	other/qsettings.h \
	other/qsettings_p.h 

SOURCES += \
	other/qdebug.cpp \
	other/qsettings.cpp

win32:SOURCES += other/qsettings_win.cpp

mac:!x11:!embedded:SOURCES += other/qsettings_mac.cpp

unix:SOURCES += other/qcrashhandler.cpp

