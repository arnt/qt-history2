DESTDIR = ../../../../bin
QT += xml network
CONFIG += qt

mac:TARGETDEPS += $$QMAKE_LIBDIR_QT/libQtDesigner.dylib

unix:!mac:!hpux*:TARGETDEPS += $$QMAKE_LIBDIR_QT/libQtDesigner.so
hpux*:TARGETDEPS += $$QMAKE_LIBDIR_QT/libQtDesigner.sl

unix:TARGETDEPS +=    ../../lib/libformeditor.a \
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
    ../lib/sdk \
    ../lib/extension \
    ../shared \
    ../components \
    ../components/formeditor \
    ../components/objectinspector \
    ../components/widgetbox \
    ../components/imagecollection \
    ../components/specialeditor \
    ../components/propertyeditor

LIBS += -L../../lib \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -limagecollection \
    -lspecialeditor \
    -luilib \
    -lsignalsloteditor \
    -lshared \
    -L../../../../lib \
    -lQtDesigner

HEADERS +=  designerapp.h \
    mainwindow.h \
    propertyeditorview.h \
    objectinspectorview.h \
    preferenceinterface.h \
    preferencedialog.h \
    designerpreferences.h \
    pluginpreferences.h \
    newformdialog.h

SOURCES +=  designerapp.cpp \
    main.cpp \
    mainwindow.cpp \
    propertyeditorview.cpp \
    objectinspectorview.cpp \
    preferencedialog.cpp \
    designerpreferences.cpp \
    pluginpreferences.cpp \
    newformdialog.cpp

RESOURCES += designer.qrc

mac {
    ICON = designer.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Designer
}

target.path=$$bins.path
INSTALLS += target
