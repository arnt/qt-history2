GUID 	 = {1ef3fd9c-446e-414e-8cac-d962eed14a7b}
TEMPLATE = app

QCONFIG += network
LIBS    += -lqassistantclient

QTDIR_build:REQUIRES = full-config table

SOURCES += main.cpp tooltip.cpp mainwindow.cpp whatsthis.cpp
HEADERS += tooltip.h mainwindow.h whatsthis.h
