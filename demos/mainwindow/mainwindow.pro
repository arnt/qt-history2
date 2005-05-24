TEMPLATE = app
HEADERS += colorswatch.h mainwindow.h toolbar.h
SOURCES += colorswatch.cpp mainwindow.cpp toolbar.cpp main.cpp
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_DEMOS]/mainwindow
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_DEMOS]/mainwindow
INSTALLS += target sources
