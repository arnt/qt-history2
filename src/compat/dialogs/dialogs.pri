# Qt compat module

HEADERS += dialogs/q3filedialog.h dialogs/qtabdialog.h
SOURCES += dialogs/q3filedialog.cpp dialogs/qtabdialog.cpp

win32:SOURCES += dialogs/q3filedialog_win.cpp
!embedded:mac:SOURCES  += dialogs/q3filedialog_mac.cpp
