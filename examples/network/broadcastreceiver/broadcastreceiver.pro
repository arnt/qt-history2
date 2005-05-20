HEADERS       = receiver.h
SOURCES       = receiver.cpp \
                main.cpp
QT           += network

# install
target.path = $$[QT_INSTALL_DATA]/examples/network/broadcastreceiver
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS broadcastreceiver.pro
sources.path = $$[QT_INSTALL_DATA]/examples/network/broadcastreceiver
INSTALLS += target sources
