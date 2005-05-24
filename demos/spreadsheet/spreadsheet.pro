TEMPLATE = app

CONFIG += qt warn_on
SOURCES += main.cpp
RESOURCES += spreadsheet.qrc

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_DEMOS]/spreadsheet
sources.files = $$SOURCES $$RESOURCES *.pro images
sources.path = $$[QT_INSTALL_DEMOS]/spreadsheet
INSTALLS += target sources
