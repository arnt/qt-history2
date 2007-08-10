HEADERS       = client.h
SOURCES       = client.cpp \
                main.cpp
QT           += network

include(../../src/src.pri)

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fortuneclient.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
INSTALLS += target sources
