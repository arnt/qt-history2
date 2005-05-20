HEADERS += httpwindow.h
SOURCES += httpwindow.cpp \
           main.cpp
QT += network

# install
target.path = $$[QT_INSTALL_DATA]/examples/network/http
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS http.pro
sources.path = $$[QT_INSTALL_DATA]/examples/network/http
INSTALLS += target sources
