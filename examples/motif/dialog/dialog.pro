TEMPLATE = app

CONFIG  += qt x11 warn_on
LIBS    += -lQtMotif

HEADERS = mainwindow.h dialog.h
SOURCES = main.cpp mainwindow.cpp dialog.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/motif/dialog
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS dialog.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/motif/dialog
INSTALLS += target sources
