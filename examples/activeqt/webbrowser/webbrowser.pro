TEMPLATE = app

CONFIG += qaxcontainer

QTDIR_build:REQUIRES = shared

SOURCES	 = main.cpp
FORMS	 = mainwindow.ui

# install
target.path = $$[QT_INSTALL_DATA]/examples/activeqt/webbrowser
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS webbrowser.pro
sources.path = $$[QT_INSTALL_DATA]/examples/activeqt/webbrowser
INSTALLS += target sources
