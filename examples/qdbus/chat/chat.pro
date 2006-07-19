TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += qdbus

# Input
HEADERS += chat.h
SOURCES += chat.cpp
FORMS += chatmainwindow.ui chatsetnickname.ui

DBUS_ADAPTORS += com.trolltech.chat.xml
DBUS_INTERFACES += com.trolltech.chat.xml
