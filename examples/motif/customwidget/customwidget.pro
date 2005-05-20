TEMPLATE = app

CONFIG  += qt x11 warn_on
LIBS    += -lQtMotif

HEADERS = mainwindow.h
SOURCES = main.cpp mainwindow.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/motif/customwidget
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS customwidget.pro
sources.path = $$[QT_INSTALL_DATA]/examples/motif/customwidget
INSTALLS += target sources
