TEMPLATE = app

CONFIG += qt warn_on
HEADERS += model.h
SOURCES += model.cpp main.cpp
RESOURCES += interview.qrc

build_all:CONFIG += release
