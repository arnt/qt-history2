win32 {
    SOURCES += paintwidget_win.cpp
}

!win32 {
    SOURCES -= paintwidget_win.cpp
}

unix {
    SOURCES += paintwidget_unix.cpp
}

macx {
    debug {
        HEADERS += debugging.h
    }
}

macx:debug {
    HEADERS += debugging.h
}
