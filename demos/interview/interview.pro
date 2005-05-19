TEMPLATE = app

CONFIG += qt warn_on
HEADERS += model.h
SOURCES += model.cpp main.cpp
RESOURCES += interview.qrc

build_all:CONFIG += release

# install
target.path = $$[QT_INSTALL_DATA]/demos/interview
sources.files = $$SOURCES $$HEADERS $$RESOURCES README *.pro images
sources.path = $$[QT_INSTALL_DATA]/demos/interview
INSTALLS += target sources
