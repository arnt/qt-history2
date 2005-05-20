HEADERS     = imagemodel.h \
              mainwindow.h \
              pixeldelegate.h
SOURCES     = imagemodel.cpp \
              main.cpp \
              mainwindow.cpp \
              pixeldelegate.cpp
RESOURCES   += images.qrc

# install
target.path = $$[QT_INSTALL_DATA]/examples/itemviews/pixelator
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/itemviews/pixelator
INSTALLS += target sources
