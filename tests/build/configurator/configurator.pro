HEADERS = dialogwidget.h configview.h menu.h qsettings.h
SOURCES = main.cpp dialogwidget.cpp configview.cpp menu.cpp qsettings.cpp
TARGET  = configurator

win32:LIBS += qtmain.lib
win32:SOURCES += qsettings_win.cpp

unix:SOURCES += qsettings_unix.cpp
