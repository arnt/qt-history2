DESTDIR = ../../../../bin
QT += xml network
CONFIG += qt

mac:TARGETDEPS += ../../../../lib/libQtDesigner.dylib \
    ../../lib/libformeditor.dylib \
    ../../lib/libobjectinspector.dylib \
    ../../lib/libimagecollection.dylib \
    ../../lib/libshared.dylib \
    ../../lib/libpropertyeditor.dylib \
    ../../lib/libspecialeditor.dylib \
    ../../lib/libsignalsloteditor.dylib

unix:!mac:TARGETDEPS += ../../../../lib/libQtDesigner.a \
    ../../lib/libformeditor.a \
    ../../lib/libobjectinspector.a \
    ../../lib/libwidgetbox.a \
    ../../lib/libimagecollection.a \
    ../../lib/libshared.a \
    ../../lib/libpropertyeditor.a \
    ../../lib/libsignalsloteditor.a

win32:TARGETDEPS += ../../../../lib/QtDesigner.lib \
    ../../lib/formeditor.lib \
    ../../lib/objectinspector.lib \
    ../../lib/widgetbox.lib \
    ../../lib/imagecollection.lib \
    ../../lib/shared.lib \
    ../../lib/propertyeditor.lib \
    ../../lib/signalsloteditor.lib

INCLUDEPATH += ../uilib \
    ../sdk \
    ../extension \
    ../shared \
    ../components \
    ../components/formeditor \
    ../components/objectinspector \
    ../components/widgetbox \
    ../components/imagecollection \
    ../components/specialeditor \
    ../components/propertyeditor

LIBS += -L../../lib \
    -lshared \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -limagecollection \
    -lspecialeditor \
    -luilib \
    -lsignalsloteditor \
    -L../../../../lib \
    -lQtDesigner

HEADERS +=  designerapp.h \
    mainwindow.h \
    propertyeditorview.h \
    objectinspectorview.h \
    newformdialog.h

SOURCES +=  designerapp.cpp \
    main.cpp \
    mainwindow.cpp \
    propertyeditorview.cpp \
    objectinspectorview.cpp \
    newformdialog.cpp

RESOURCES += designer.qrc

mac {
    RC_FILE      = designer.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Designer
}
