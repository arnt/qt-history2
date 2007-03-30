include(../src/components/buddyeditor/buddyeditor.pri)
include(../src/components/component.pri)
include(../src/components/formeditor/formeditor.pri)
include(../src/components/objectinspector/objectinspector.pri)
include(../src/components/propertyeditor/propertyeditor.pri)
include(../src/components/signalsloteditor/signalsloteditor.pri)
include(../src/components/tabordereditor/tabordereditor.pri)
include(../src/components/taskmenu/taskmenu.pri)
include(../src/components/widgetbox/widgetbox.pri)
include(../src/lib/extension/extension.pri)
include(../src/lib/sdk/sdk.pri)
include(../src/lib/shared/shared.pri)
include(../src/lib/uilib/uilib.pri)
include(../src/plugins/plugins.pri)

# Include those manually as they do not contain any directory specification
APP_DIR=../src/designer
SOURCES += $$APP_DIR/main.cpp \
    $$APP_DIR/qdesigner.cpp \
    $$APP_DIR/qdesigner_toolwindow.cpp \
    $$APP_DIR/qdesigner_formwindow.cpp \
    $$APP_DIR/qdesigner_workbench.cpp \
    $$APP_DIR/qdesigner_settings.cpp \
    $$APP_DIR/qdesigner_server.cpp \
    $$APP_DIR/qdesigner_widgetbox.cpp \
    $$APP_DIR/qdesigner_propertyeditor.cpp \
    $$APP_DIR/qdesigner_objectinspector.cpp \
    $$APP_DIR/qdesigner_actioneditor.cpp \
    $$APP_DIR/qdesigner_actions.cpp \
    $$APP_DIR/qdesigner_resourceeditor.cpp \
    $$APP_DIR/saveformastemplate.cpp \
    $$APP_DIR/newform.cpp \
    $$APP_DIR/versiondialog.cpp \
    $$APP_DIR/qdesigner_signalsloteditor.cpp \
    $$APP_DIR/plugindialog.cpp \
    $$APP_DIR/formwindowsettings.cpp \
    $$APP_DIR/extra/cursor.cpp \
    $$APP_DIR/extra/fov.cpp \
    $$APP_DIR/extra/itemdialog.cpp \
    $$APP_DIR/extra/oubliette.cpp \
    $$APP_DIR/extra/oublietteplan.cpp \
    $$APP_DIR/extra/oublietteresource.cpp \
    $$APP_DIR/extra/oublietteview.cpp \
    $$APP_DIR/preferencesdialog.cpp \
    $$APP_DIR/preferences.cpp

FORMS += $$APP_DIR/newform.ui \
    $$APP_DIR/plugindialog.ui \
    $$APP_DIR/saveformastemplate.ui \
    $$APP_DIR/formwindowsettings.ui \
    $$APP_DIR/preferencesdialog.ui

SOURCES += ../../shared/fontpanel/fontpanel.cpp

TRANSLATIONS=$$[QT_INSTALL_TRANSLATIONS]/designer_de.ts $$[QT_INSTALL_TRANSLATIONS]/designer_untranslated.ts
error("This is a dummy profile to be used for translations ONLY.")
