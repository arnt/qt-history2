HEADERS = dropsitewidget.h \
          dropsitewindow.h
SOURCES = dropsitewidget.cpp \
          dropsitewindow.cpp \
          main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/dropsite
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro images
sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/dropsite
INSTALLS += target sources

