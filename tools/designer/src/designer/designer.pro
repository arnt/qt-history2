DESTDIR = ../../../../bin
QT += xml network
CONFIG += qt

mac:TARGETDEPS += ../../../../lib/libQtDesigner.dylib \
    ../../lib/libformeditor.lib \
    ../../lib/libobjectinspector.lib \
    ../../lib/libimagecollection.lib \
    ../../lib/libshared.lib \
    ../../lib/libpropertyeditor.lib \
    ../../lib/libspecialeditor.lib \
    ../../lib/libsignalsloteditor.lib

unix:!mac:TARGETDEPS += ../../../../lib/libQtDesigner.so \
    ../../lib/libformeditor.a \
    ../../lib/libobjectinspector.a \
    ../../lib/libwidgetbox.a \
    ../../lib/libimagecollection.a \
    ../../lib/libshared.a \
    ../../lib/libpropertyeditor.a \
    ../../lib/libsignalsloteditor.a

win32:TARGETDEPS += ../../../../lib/QtDesigner.dll \
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
    -lQtDesigner \
    -lshared \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -limagecollection \
    -lspecialeditor \
    -lshared \
    -luilib \
    -lsignalsloteditor

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
