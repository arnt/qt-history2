
HEADERS += uic.h \
           ui4.h \
           treewalker.h \
           writedeclaration.h \
           writeicondeclaration.h \
           option.h \
           driver.h \
           validator.h \
           writeincludes.h \
           writeinitialization.h \
           writeiconinitialization.h \
           writeicondata.h \
           blockingprocess.h

SOURCES += uic.cpp \
           main.cpp \
           treewalker.cpp \
           writedeclaration.cpp \
           writeicondeclaration.cpp \
           driver.cpp \
           validator.cpp \
           writeincludes.cpp \
           writeinitialization.cpp \
           writeiconinitialization.cpp \
           writeicondata.cpp \
           blockingprocess.cpp

INCLUDEPATH += .
QT = xml core
CONFIG += warn_on console no_batch
TEMPLATE = app
DEFINES += QT_KEYWORDS
TARGET = uic4
DESTDIR = ../../../bin
