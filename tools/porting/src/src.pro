TEMPLATE = app
CONFIG -= moc
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
TARGET =qt3to4
DESTDIR = ../../../bin

HEADERS +=  smallobject.h             \
            tokenengine.h             \
            tokenizer.h               \
            rpplexer.h                \
            rpp.h                     \
            rpptreeevaluator.h        \
            rppexpressionbuilder.h    \
            preprocessorcontrol.h     \
            replacetoken.h            \
            tokenreplacements.h       \
            textreplacement.h         \
            portingrules.h            \
            qtsimplexml.h             \
            projectporter.h           \
            proparser.h               \
            fileporter.h              \
            logger.h                  \
            tokens.h                  \
            filewriter.h              \
            cpplexer.h                \
            smallobject.h             \
            ast.h                     \
            errors.h                  \
            parser.h                  \
            translationunit.h         \
            treewalker.h              \
            semantic.h                \
            codemodel.h               \
            codemodelwalker.h         \
            codemodelattributes.h

SOURCES += port.cpp                  \
           smallobject.cpp           \
           tokenengine.cpp           \
           tokenizer.cpp             \
           rpplexer.cpp              \
           rpp.cpp                   \
           rpptreeevaluator.cpp      \
           rppexpressionbuilder.cpp  \
           preprocessorcontrol.cpp   \
           replacetoken.cpp          \
           tokenreplacements.cpp     \
           textreplacement.cpp       \
           portingrules.cpp          \
           qtsimplexml.cpp           \
           projectporter.cpp         \
           proparser.cpp             \
           fileporter.cpp            \
           logger.cpp                \
           filewriter.cpp            \
           cpplexer.cpp              \
           ast.cpp                   \
           errors.cpp                \
           parser.cpp                \
           translationunit.cpp       \
           treewalker.cpp            \
           semantic.cpp              \
           codemodel.cpp             \
           codemodelwalker.cpp       \
           codemodelattributes.cpp


OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared
QT = xml core

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

RULESXML = q3porting.xml
RULESXML.files = q3porting.xml
RULESXML.path = $$[QT_INSTALL_DATA]/qt3to4
INSTALLS += RULESXML
