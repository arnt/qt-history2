TEMPLATE = app

SOURCES += main.cpp tooltip.cpp mainwindow.cpp whatsthis.cpp
HEADERS += tooltip.h mainwindow.h whatsthis.h

win32:LIBS += qassistantclient.lib
unix:LIBS += -lqassistantclient

