HEADERS       = mainwindow.h
SOURCES       = mainwindow.cpp \
                main.cpp

# install
target.path = $$[QT_INSTALL_DATA]/examples/mainwindows/menus
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS menus.pro
sources.path = $$[QT_INSTALL_DATA]/examples/mainwindows/menus
INSTALLS += target sources
