
QTDIR       = $$QT_SOURCE_TREE

CONFIG      += designer plugin debug_and_release
TEMPLATE    = lib
DESTDIR     = $$QT_BUILD_TREE/plugins/designer

SHARED_FOLDER = ../shared
include(../shared/shared.pri)

DEMO_DEFORM_DIR = $$QTDIR/demos/deform
DEMO_AFFINE_DIR = $$QTDIR/demos/affine
DEMO_GRADIENT_DIR = $$QTDIR/demos/gradients
DEMO_STROKE_DIR = $$QTDIR/demos/pathstroke
DEMO_COMPOSITION_DIR = $$QTDIR/demos/composition

INCLUDEPATH += $$DEMO_DEFORM_DIR $$DEMO_AFFINE_DIR $$DEMO_GRADIENT_DIR $$DEMO_STROKE_DIR $$DEMO_COMPOSITION_DIR

SOURCES = plugin.cpp \
	$$DEMO_COMPOSITION_DIR/composition.cpp \
        $$DEMO_AFFINE_DIR/xform.cpp \
        $$DEMO_DEFORM_DIR/pathdeform.cpp \
        $$DEMO_GRADIENT_DIR/gradients.cpp \
        $$DEMO_STROKE_DIR/pathstroke.cpp \
             

HEADERS = \
	$$DEMO_COMPOSITION_DIR/composition.h \
        $$DEMO_AFFINE_DIR/xform.h \
        $$DEMO_DEFORM_DIR/pathdeform.h \
        $$DEMO_GRADIENT_DIR/gradients.h \
        $$DEMO_STROKE_DIR/pathstroke.h \

              
# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/customwidgetplugin
INSTALLS += target sources

RESOURCES += arthur_plugin.qrc

win32-msvc.net|win32-msvc {
	QMAKE_CFLAGS += /Zm500
	QMAKE_CXXFLAGS += /Zm500
}

