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


# Include ActiveQt plugin
SOURCES += ../src/plugins/activeqt/plugin.cpp \
           ../src/plugins/activeqt/activeqt_extrainfo.cpp \
           ../../../src/activeqt/shared/qaxtypes.cpp

HEADERS += ../src/plugins/activeqt/activeqt_extrainfo.h \
           ../../../src/activeqt/shared/qaxtypes.h


# Include Qt3Support plugin

SOURCES += ../src/plugins/widgets/qt3supportwidgets.cpp
HEADERS += ../src/plugins/widgets/q3buttongroup/q3buttongroup_plugin.h \
           ../src/plugins/widgets/q3frame/q3frame_plugin.h \
           ../src/plugins/widgets/q3groupbox/q3groupbox_plugin.h \
           ../src/plugins/widgets/q3iconview/q3iconview_extrainfo.h \
           ../src/plugins/widgets/q3iconview/q3iconview_plugin.h \
           ../src/plugins/widgets/q3listview/q3listview_extrainfo.h \
           ../src/plugins/widgets/q3listview/q3listview_plugin.h \
           ../src/plugins/widgets/q3mainwindow/q3mainwindow_container.h \
           ../src/plugins/widgets/q3mainwindow/q3mainwindow_plugin.h \
           ../src/plugins/widgets/q3toolbar/q3toolbar_extrainfo.h \
           ../src/plugins/widgets/q3toolbar/q3toolbar_plugin.h \
           ../src/plugins/widgets/q3widgetstack/q3widgetstack_container.h \
           ../src/plugins/widgets/q3widgetstack/q3widgetstack_plugin.h \
           ../src/plugins/widgets/q3widgetstack/qdesigner_q3widgetstack_p.h \
           ../src/plugins/widgets/q3wizard/q3wizard_container.h \
           ../src/plugins/widgets/q3wizard/q3wizard_plugin.h \
           ../src/plugins/widgets/q3listbox/q3listbox_extrainfo.h \
           ../src/plugins/widgets/q3listbox/q3listbox_plugin.h \
           ../src/plugins/widgets/q3table/q3table_extrainfo.h \
           ../src/plugins/widgets/q3table/q3table_plugin.h \
           ../src/plugins/widgets/q3textedit/q3textedit_extrainfo.h \
           ../src/plugins/widgets/q3textedit/q3textedit_plugin.h \
           ../src/plugins/widgets/q3dateedit/q3dateedit_plugin.h \
           ../src/plugins/widgets/q3timeedit/q3timeedit_plugin.h \
           ../src/plugins/widgets/q3datetimeedit/q3datetimeedit_plugin.h \
           ../src/plugins/widgets/q3progressbar/q3progressbar_plugin.h \
           ../src/plugins/widgets/q3textbrowser/q3textbrowser_plugin.h

SOURCES += ../src/plugins/widgets/q3buttongroup/q3buttongroup_plugin.cpp \
           ../src/plugins/widgets/q3frame/q3frame_plugin.cpp \
           ../src/plugins/widgets/q3groupbox/q3groupbox_plugin.cpp \
           ../src/plugins/widgets/q3iconview/q3iconview_extrainfo.cpp \
           ../src/plugins/widgets/q3iconview/q3iconview_plugin.cpp \
           ../src/plugins/widgets/q3listview/q3listview_extrainfo.cpp \
           ../src/plugins/widgets/q3listview/q3listview_plugin.cpp \
           ../src/plugins/widgets/q3mainwindow/q3mainwindow_container.cpp \
           ../src/plugins/widgets/q3mainwindow/q3mainwindow_plugin.cpp \
           ../src/plugins/widgets/q3toolbar/q3toolbar_extrainfo.cpp \
           ../src/plugins/widgets/q3toolbar/q3toolbar_plugin.cpp \
           ../src/plugins/widgets/q3widgetstack/q3widgetstack_container.cpp \
           ../src/plugins/widgets/q3widgetstack/q3widgetstack_plugin.cpp \
           ../src/plugins/widgets/q3widgetstack/qdesigner_q3widgetstack.cpp \
           ../src/plugins/widgets/q3wizard/q3wizard_container.cpp \
           ../src/plugins/widgets/q3wizard/q3wizard_plugin.cpp \
           ../src/plugins/widgets/q3listbox/q3listbox_extrainfo.cpp \
           ../src/plugins/widgets/q3listbox/q3listbox_plugin.cpp \
           ../src/plugins/widgets/q3table/q3table_extrainfo.cpp \
           ../src/plugins/widgets/q3table/q3table_plugin.cpp \
           ../src/plugins/widgets/q3textedit/q3textedit_extrainfo.cpp \
           ../src/plugins/widgets/q3textedit/q3textedit_plugin.cpp \
           ../src/plugins/widgets/q3dateedit/q3dateedit_plugin.cpp \
           ../src/plugins/widgets/q3timeedit/q3timeedit_plugin.cpp \
           ../src/plugins/widgets/q3datetimeedit/q3datetimeedit_plugin.cpp \
           ../src/plugins/widgets/q3progressbar/q3progressbar_plugin.cpp \
           ../src/plugins/widgets/q3textbrowser/q3textbrowser_plugin.cpp


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

TRANSLATIONS=$$[QT_INSTALL_TRANSLATIONS]/designer_de.ts \
             $$[QT_INSTALL_TRANSLATIONS]/designer_ja.ts \
             $$[QT_INSTALL_TRANSLATIONS]/designer_untranslated.ts 
error("This is a dummy profile to be used for translations ONLY.")
