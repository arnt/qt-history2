HEADERS       = server.h
SOURCES       = server.cpp \
                main.cpp
QT           += network

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneserver
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fortuneserver.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneserver
INSTALLS += target sources
