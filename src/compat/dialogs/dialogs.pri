# Qt compat module

HEADERS += dialogs/q3filedialog.h
SOURCES += dialogs/q3filedialog.cpp

win32:SOURCES += dialogs/q3filedialog_win.cpp
!embedded:mac:SOURCES  += dialogs/q3filedialog_mac.cpp
