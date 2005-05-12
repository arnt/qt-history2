CONFIG      += designer plugin
QT          += xml
TEMPLATE    = lib

DESTDIR     = $$[QT_INSTALL_PLUGINS]/designer
HEADERS     = worldtimeclock.h \
              worldtimeclockplugin.h
SOURCES     = worldtimeclock.cpp \
              worldtimeclockplugin.cpp
