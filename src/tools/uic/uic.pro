
HEADERS += uic.h \
           ui4.h \
           treewalker.h \
           writedeclaration.h \
           option.h \
           driver.h \
           validator.h \
           writeincludes.h \
           writeinitialization.h \
           blockingprocess.h

SOURCES += uic.cpp \
           main.cpp \
           treewalker.cpp \
           writedeclaration.cpp \
           driver.cpp \
           validator.cpp \
           writeincludes.cpp \
           writeinitialization.cpp \
           blockingprocess.cpp

INCLUDEPATH += .
QT = xml core
CONFIG += debug warn_on console no_batch
TEMPLATE = app
DEFINES += QT_KEYWORDS
TARGET = uic4
DESTDIR = ../../../bin
