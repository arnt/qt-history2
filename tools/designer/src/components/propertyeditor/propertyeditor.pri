
INCLUDEPATH += $$PWD \
    $$QT_BUILD_TREE/tools/designer/src/components/propertyeditor

FORMS += $$PWD/paletteeditor.ui \
    $$PWD/previewwidget.ui

HEADERS += $$PWD/propertyeditor.h \
    $$PWD/paletteeditor.h \
    $$PWD/paletteeditorbutton.h \
    $$PWD/previewwidget.h \
    $$PWD/previewframe.h \
    $$PWD/styledbutton.h

SOURCES += $$PWD/propertyeditor.cpp \
    $$PWD/paletteeditor.cpp \
    $$PWD/paletteeditorbutton.cpp \
    $$PWD/previewwidget.cpp \
    $$PWD/previewframe.cpp \
    $$PWD/styledbutton.cpp

HEADERS += $$PWD/qpropertyeditor.h \
    $$PWD/qpropertyeditor_items_p.h \
    $$PWD/qpropertyeditor_model_p.h \
    $$PWD/qpropertyeditor_delegate_p.h \
    $$PWD/propertyeditor_global.h \
    $$PWD/flagbox_p.h \
    $$PWD/flagbox_model_p.h \
    $$PWD/defs.h

SOURCES += $$PWD/qpropertyeditor.cpp \
    $$PWD/qpropertyeditor_items.cpp \
    $$PWD/qpropertyeditor_model.cpp \
    $$PWD/qpropertyeditor_delegate.cpp \
    $$PWD/flagbox.cpp \
    $$PWD/flagbox_model.cpp \
    $$PWD/defs.cpp

