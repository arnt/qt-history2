CONFIG      += designer plugin
TEMPLATE    = lib
DESTDIR     = $$[QT_INSTALL_PLUGINS]/designer

HEADERS     = analogclock.h \
              customwidgetplugin.h
SOURCES     = analogclock.cpp \
              customwidgetplugin.cpp
