HEADERS       = sender.h
SOURCES       = sender.cpp \
                main.cpp
QT           += network

# install
target.path = $$[QT_INSTALL_DATA]/examples/network/broadcastsender
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS broadcastsender.pro
sources.path = $$[QT_INSTALL_DATA]/examples/network/broadcastsender
INSTALLS += target sources
