HEADERS       = mainwindow.h
SOURCES       = main.cpp \
                mainwindow.cpp
RESOURCES     = sdi.qrc

# install
target.path = $$[QT_INSTALL_DATA]/examples/mainwindows/sdi
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS sdi.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/mainwindows/sdi
INSTALLS += target sources
