
QT += xml

IDE_ROOT = $$PWD/../../..

INCLUDEPATH += \
    . \
    $$PWD \
    $$PWD/../../../src/sdk \
    $$PWD/../../../src/extension \
    $$PWD/../../../src/uilib \
    $$PWD/../../../src/shared \
    $$PWD/../../../src/components \
    $$PWD/../../../src/components/formeditor \
    $$PWD/../../../src/components/widgetbox \
    $$PWD/../../../src/components/objectinspector

DEPENDPATH += $$INCLUDEPATH

include($(QTESTDIR)/include/qtest.pri)

HEADERS += \
    $$PWD/ideapplication.h

SOURCES += \
    $$PWD/ideapplication.cpp

LIBS += \
    -L$$IDE_ROOT/lib \
    -lsdk \
    -lextension \
    -luilib \
    -lshared \
    -lformeditor \
    -lpropertyeditor \
    -lwidgetbox \
    -lobjectinspector

