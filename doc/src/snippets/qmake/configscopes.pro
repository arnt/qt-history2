SOURCES = main.cpp
CONFIG += debug

debug {
    TARGET = application-debug
}

release {
    TARGET = application
}

CONFIG(opengl) {
    message(Building with OpenGL support.)
} else {
    message(OpenGL support is not available.)
}
