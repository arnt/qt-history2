
DESTDIR = ../../../../bin
QT += xml network
CONFIG += qt assistant
build_all:CONFIG += release

INCLUDEPATH += \
    ../lib/sdk \
    ../lib/extension \
    ../lib/shared \
    ../lib/uilib \
    ../components \
    extra \
    ../components/formeditor \
    ../components/objectinspector \
    ../components/widgetbox \
    ../components/propertyeditor \
    ../components/taskmenu \
    ../components/resourceeditor

LIBS += -L../../lib \
    -L../../../../lib \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -lsignalsloteditor \
    -lbuddyeditor \
    -ltabordereditor \
    -ltaskmenu \
    -lresourceeditor \
    -lQtDesigner

RESOURCES += designer.qrc


TARGET = designer

HEADERS += \
    qdesigner.h \
    qdesigner_toolwindow.h \
    qdesigner_formwindow.h \
    qdesigner_workbench.h \
    qdesigner_settings.h \
    qdesigner_server.h \
    qdesigner_widgetbox.h \
    qdesigner_propertyeditor.h \
    qdesigner_objectinspector.h \
    qdesigner_actions.h \
    qdesigner_resourceeditor.h \
    preferenceinterface.h \
    saveformastemplate.h \
    newform.h \
    versiondialog.h \
    qdesigner_signalsloteditor.h \
    plugindialog.h \
    extra/cursor.h \
    extra/fov.h \
    extra/itemdialog.h \
    extra/oubliette.h \
    extra/oublietteplan.h \
    extra/oublietteview.h

SOURCES += main.cpp \
    qdesigner.cpp \
    qdesigner_toolwindow.cpp \
    qdesigner_formwindow.cpp \
    qdesigner_workbench.cpp \
    qdesigner_settings.cpp \
    qdesigner_server.cpp \
    qdesigner_widgetbox.cpp \
    qdesigner_propertyeditor.cpp \
    qdesigner_objectinspector.cpp \
    qdesigner_actions.cpp \
    qdesigner_plugins.cpp \
    qdesigner_resourceeditor.cpp \
    saveformastemplate.cpp \
    newform.cpp \
    versiondialog.cpp \
    qdesigner_signalsloteditor.cpp \
    plugindialog.cpp \
    extra/cursor.cpp \
    extra/fov.cpp \
    extra/itemdialog.cpp \
    extra/oubliette.cpp \
    extra/oublietteplan.cpp \
    extra/oublietteresource.cpp \
    extra/oublietteview.cpp


PRECOMPILED_HEADER=qdesigner_pch.h

FORMS += \
    newform.ui \
    saveformastemplate.ui

win32 {
   RC_FILE	= designer.rc
}

mac {
    ICON = designer.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Designer
}

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

include(../sharedcomponents.pri)

unix:!mac:LIBS += -lm
