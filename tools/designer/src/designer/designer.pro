
DESTDIR = ../../../../bin
QT += xml network compat
CONFIG += qt depend_prl assistant

INCLUDEPATH += ../uilib \
    ../lib/sdk \
    ../lib/extension \
    ../shared \
    ../components \
    ../components/formeditor \
    ../components/objectinspector \
    ../components/widgetbox \
    ../components/imagecollection \
    ../components/propertyeditor \
    ../components/taskmenu \
    ../../../assistant/lib

LIBS += -L../../lib \
    -L../../../../lib \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -limagecollection \
    -luilib \
    -lsignalsloteditor \
    -lbuddyeditor \
    -lshared \
    -ltaskmenu \
    -lQtDesigner

RESOURCES += designer.qrc

include(designer.pri)

mac {
    ICON = designer.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Designer
}

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

include(../sharedcomponents.pri)

unix:!mac:LIBS += -lm
