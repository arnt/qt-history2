SOURCES  += plugin.cpp ../widget/filechooser.cpp
HEADERS  += plugin.h ../widget/filechooser.h
DESTDIR   = ../../../../../plugins/designer
TARGET    = filechooser

target.path=$$plugins.path

INSTALLS    += target
TEMPLATE     = lib
CONFIG      += qt warn_on release plugin
INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces
DBFILE       = plugin.db
LANGUAGE     = C++
