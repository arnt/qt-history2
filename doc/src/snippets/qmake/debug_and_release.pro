CONFIG += debug_and_release

CONFIG(debug, debug|release) {
    TARGET = debug_binary
} else {
    TARGET = release_binary
}

CONFIG += build_all
