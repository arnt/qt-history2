TEMPLATE = app
CONFIG -= moc
CONFIG += console
DEPENDPATH += .
TARGET =../../../bin/qt3to4

HEADERS +=  replacetoken.h            \
            tokenreplacements.h       \
            textreplacement.h         \
            rulesfromxml.h            \
            qtsimplexml.h             \
            projectporter.h           \
            proparser.h               \
            fileporter.h              \
            logger.h                  \
            lexer.h                   \
            tokens.h                  \
            filewriter.h              

SOURCES += port.cpp                  \
           replacetoken.cpp          \
           tokenreplacements.cpp     \
           textreplacement.cpp       \
           rulesfromxml.cpp          \
           qtsimplexml.cpp           \
           projectporter.cpp         \
           proparser.cpp             \
           fileporter.cpp            \
           logger.cpp                \
           lexer.cpp                 \
           filewriter.cpp              

OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared
QT += compat xml
