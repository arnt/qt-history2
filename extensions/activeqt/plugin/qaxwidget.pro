TEMPLATE = lib

IDEDIR = $$QT_BUILD_TREE/tools/designer
DESTDIR = $$QT_BUILD_TREE/plugins/designer

INCLUDEPATH += \
    $$IDEDIR/src/lib/sdk \
    $$IDEDIR/src/lib/extension \
    $$IDEDIR/src/uilib \
    $$IDEDIR/src/shared

# Input
SOURCES += plugin.cpp
CONFIG += qt warn_on qaxcontainer


CONFIG(debug, debug|release) {
    LIBS += $$IDEDIR/lib/sharedd.lib
} else {
    LIBS += $$IDEDIR/lib/shared.lib
}
