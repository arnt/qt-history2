TEMPLATE        = lib
CONFIG          += qt warn_on plugin
win32:DEFINES   += QT_DLL QT_PLUGIN_STYLE_LIGHT

HEADERS         = lightstyle.h

SOURCES         = main.cpp \
                  lightstyle.cpp

unix:OBJECTS_DIR        = .obj
win32:OBJECTS_DIR       = obj

TARGET          = lightstyle
DESTDIR         = ../../plugins/styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target

