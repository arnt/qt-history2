TEMPLATE = app

SOURCES += main.cpp tooltip.cpp mainwindow.cpp whatsthis.cpp
HEADERS += tooltip.h mainwindow.h whatsthis.h

LIBS += -lqassistantclient

QTDIR_build:REQUIRES=full-config
