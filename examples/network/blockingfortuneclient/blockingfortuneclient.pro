HEADERS       = blockingclient.h \
                fortunethread.h
SOURCES       = blockingclient.cpp \
                main.cpp \
                fortunethread.cpp
QT           += network

# install
target.path = $$[QT_INSTALL_DATA]/examples/network/blockingfortuneclient
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS blockingfortuneclient.pro
sources.path = $$[QT_INSTALL_DATA]/examples/network/blockingfortuneclient
INSTALLS += target sources
