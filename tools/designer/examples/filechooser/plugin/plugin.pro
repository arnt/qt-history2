SOURCES  += plugin.cpp ../widget/filechooser.cpp
HEADERS  += plugin.h ../widget/filechooser.h
DESTDIR   = ../../../../../plugins/designer
TARGET    = filechooser

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS    += target
TEMPLATE     = lib
CONFIG      += qt warn_on release plugin
INCLUDEPATH += $(QTDIR)/tools/designer/interfaces
DBFILE       = plugin.db
LANGUAGE     = C++
