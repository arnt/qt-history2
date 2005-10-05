SOURCES = main.cpp
CONFIG += debug

debug {
    TARGET = application-debug
}

release {
    TARGET = application
}

CONFIG(debug, debug|release) {
    message(Building in debug mode.)
} else {
    message(Building in release mode. Ignoring debug if it is available.)
}
