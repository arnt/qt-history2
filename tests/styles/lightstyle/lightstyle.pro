TEMPLATE        = lib
CONFIG          += qt warn_on plugin

contains(styles,windows) {
    HEADERS         = lightstyle-v2.h lightstyle-v3.h

    SOURCES         = main.cpp lightstyle-v2.cpp lightstyle-v3.cpp

    unix:OBJECTS_DIR        = .obj
    win32:OBJECTS_DIR       = obj

    TARGET          = lightstyle
    DESTDIR         = ../../../plugins/styles

    target.path=$$plugins.path/styles
    isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
    INSTALLS += target
}

!contains(styles,windows) {
    message("The Light style requires the Windows style.")
}
