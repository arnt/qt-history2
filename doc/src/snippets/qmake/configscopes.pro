SOURCES = main.cpp
CONFIG += opengl

opengl {
    TARGET = application-gl
} else {
    TARGET = application
}

CONFIG(opengl) {
    message(Building with OpenGL support.)
} else {
    message(OpenGL support is not available.)
}
