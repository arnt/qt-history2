
DESTDIR = ../../../../bin
QT += xml network qt3support
CONFIG += qt depend_prl assistant

INCLUDEPATH += ../uilib \
    ../lib/sdk \
    ../lib/extension \
    ../shared \
    ../components \
    ../components/formeditor \
    ../components/objectinspector \
    ../components/widgetbox \
    ../components/propertyeditor \
    ../components/taskmenu \
    ../../../assistant/lib

LIBS += -L../../lib \
    -L../../../../lib \
    -lformeditor \
    -lobjectinspector \
    -lpropertyeditor \
    -lwidgetbox \
    -luilib \
    -lsignalsloteditor \
    -lbuddyeditor \
    -ltaskmenu \
    -lshared \
    -lQtDesigner

RESOURCES += designer.qrc


TARGET = designer

HEADERS += \
    qdesigner.h \
    qdesigner_mainwindow.h \
    qdesigner_toolwindow.h \
    qdesigner_formwindow.h \
    qdesigner_workbench.h \
    qdesigner_settings.h \
    qdesigner_session.h \
    qdesigner_server.h \
    qdesigner_widgetbox.h \
    qdesigner_propertyeditor.h \
    qdesigner_objectinspector.h \
    qdesigner_integration.h \
    qdesigner_actions.h \
    qdesigner_preferences.h \
    preferenceinterface.h \
    pluginpreferences.h \
    preferencedialog.h \
    newform.h

SOURCES += main.cpp \
    qdesigner.cpp \
    qdesigner_mainwindow.cpp \
    qdesigner_toolwindow.cpp \
    qdesigner_formwindow.cpp \
    qdesigner_workbench.cpp \
    qdesigner_settings.cpp \
    qdesigner_session.cpp \
    qdesigner_server.cpp \
    qdesigner_widgetbox.cpp \
    qdesigner_propertyeditor.cpp \
    qdesigner_objectinspector.cpp \
    qdesigner_integration.cpp \
    qdesigner_preferences.cpp \
    qdesigner_actions.cpp \
    pluginpreferences.cpp \
    preferencedialog.cpp \
    newform.cpp

FORMS += newform.ui

mac {
    ICON = designer.icns
    QMAKE_INFO_PLIST = Info_mac.plist
    TARGET = Designer
}

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

include(../sharedcomponents.pri)

unix:!mac:LIBS += -lm
