# Qt compat module

HEADERS += \
	dialogs/q3filedialog.h \
	dialogs/q3tabdialog.h \
	dialogs/qtabdialog.h \
	dialogs/qwizard.h
SOURCES += \
	dialogs/q3filedialog.cpp \
	dialogs/q3tabdialog.cpp \
	dialogs/qwizard.cpp

win32:SOURCES += dialogs/q3filedialog_win.cpp
!embedded:mac:SOURCES  += dialogs/q3filedialog_mac.cpp
