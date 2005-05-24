HEADERS     = configdialog.h \
              pages.h
SOURCES     = configdialog.cpp \
              main.cpp \
              pages.cpp
RESOURCES   += configdialog.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/dialogs/configdialog
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro images
sources.path = $$[QT_INSTALL_EXAMPLES]/dialogs/configdialog
INSTALLS += target sources
