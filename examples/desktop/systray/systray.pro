HEADERS = mainwindow.h
SOURCES = mainwindow.cpp \
          main.cpp
RESOURCES = systray.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/desktop/systray
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS systray.pro resources
sources.path = $$[QT_INSTALL_EXAMPLES]/desktop/systray
INSTALLS += target sources

