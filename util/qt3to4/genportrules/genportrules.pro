HEADERS += ast.h \
           errors.cpp \
           lexer.h \
           list.h \
           parser.h \
           smallobject.h \
           tokens.h \
           treedump.h \
           treewalker.h \
           qtsimplexml.h

SOURCES += ast.cpp \
           errors.cpp \
           lexer.cpp \
           list.cpp \
           parser.cpp \
           smallobject.cpp \
           treedump.cpp \
           treewalker.cpp \
           qtsimplexml.cpp

TEMPLATE = app
CONFIG -= moc
DEPENDPATH += .
TARGET = genportrules

SOURCES += genportrules.cpp  genqtsymbols.cpp genclassnames.cpp gendocrules.cpp
HEADERS += genqtsymbols.h genclassnames.h gendocrules.h
CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared
QT += xml qt3support
