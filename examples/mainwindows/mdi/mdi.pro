HEADERS       = mainwindow.h \
                mdichild.h
SOURCES       = main.cpp \
                mainwindow.cpp \
                mdichild.cpp
RESOURCES     = mdi.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/mainwindows/mdi
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS mdi.pro images
sources.path = $$[QT_INSTALL_EXAMPLES]/mainwindows/mdi
INSTALLS += target sources
