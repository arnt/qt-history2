HEADERS += chatdialog.h \
           client.h \
           connection.h \
           peermanager.h
SOURCES += chatdialog.cpp \
           client.cpp \
           connection.cpp \
           main.cpp \
           peermanager.cpp
FORMS += chat.ui
QT += network

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/chat
sources.files = $$SOURCES $$HEADERS $$RESOURCES chat.pro *.chat
sources.path = $$[QT_INSTALL_EXAMPLES]/network/chat
INSTALLS += target sources
