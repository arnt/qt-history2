TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += qdbus

# Input
HEADERS += chat.h chatadaptor.h chatinterface.h
FORMS += chatmainwindow.ui chatsetnickname.ui
SOURCES += chat.cpp chatadaptor.cpp chatinterface.cpp
