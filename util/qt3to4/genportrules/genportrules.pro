RXXPATH = $$PWD/../../..
INCLUDEPATH += ../../../ \
            ../common

HEADERS += $$RXXPATH/ast.h \
           $$RXXPATH/driver.h \
           $$RXXPATH/errors.h \
           $$RXXPATH/klocale.h \
           $$RXXPATH/lexer.h \
           $$RXXPATH/list.h \
           $$RXXPATH/parser.h \
           $$RXXPATH/preprocessor.h \
           $$RXXPATH/smallobject.h \
           $$RXXPATH/tokens.h \
           $$RXXPATH/treedump.h \
           $$RXXPATH/treewalker.h \

SOURCES += $$RXXPATH/ast.cpp \
           $$RXXPATH/driver.cpp \
           $$RXXPATH/errors.cpp \
           $$RXXPATH/lexer.cpp \
           $$RXXPATH/list.cpp \
           $$RXXPATH/parser.cpp \
           $$RXXPATH/preprocessor.cpp \
           $$RXXPATH/smallobject.cpp \
           $$RXXPATH/treedump.cpp \
           $$RXXPATH/treewalker.cpp \

SOURCES +=  ../common/qtsimplexml.cpp
HEADERS +=  ../common/qtsimplexml.h

TEMPLATE = app
CONFIG -= moc
DEPENDPATH += .
TARGET =../../genportrules

SOURCES += genportrules.cpp  genqtsymbols.cpp genclassnames.cpp gendocrules.cpp
HEADERS += genqtsymbols.h genclassnames.h gendocrules.h
INCLUDES +=
CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared
QT += xml compat