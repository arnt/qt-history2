TEMPLATE = app
CONFIG -= moc
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
TARGET =qt3to4
DESTDIR = ../../../bin

HEADERS +=  replacetoken.h            \
            tokenreplacements.h       \
            textreplacement.h         \
            portingrules.h            \
            qtsimplexml.h             \
            projectporter.h           \
            proparser.h               \
            fileporter.h              \
            logger.h                  \
            tokens.h                  \
            filewriter.h

SOURCES += port.cpp                  \
           replacetoken.cpp          \
           tokenreplacements.cpp     \
           textreplacement.cpp       \
           portingrules.cpp          \
           qtsimplexml.cpp           \
           projectporter.cpp         \
           proparser.cpp             \
           fileporter.cpp            \
           logger.cpp                \
           filewriter.cpp

OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared
QT = xml core

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

RULESXML = q3porting.xml
RULESXML.files = q3porting.xml
RULESXML.path = $$[QT_INSTALL_DATA]/qt3to4
INSTALLS += RULESXML
