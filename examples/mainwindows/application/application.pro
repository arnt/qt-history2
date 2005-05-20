HEADERS       = mainwindow.h
SOURCES       = main.cpp \
                mainwindow.cpp
RESOURCES     = application.qrc

# install
target.path = $$[QT_INSTALL_DATA]/examples/mainwindows/application
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS application.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/mainwindows/application
INSTALLS += target sources
