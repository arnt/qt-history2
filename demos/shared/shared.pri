INCLUDEPATH += $$SHARED_FOLDER

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
contains(CONFIG, debug_and_release_target) {    
    CONFIG(debug, debug|release) { 
	LIBS+=-L$$SHARED_FOLDER/debug
    } else {
	LIBS+=-L$$SHARED_FOLDER/release
    }
} else {
    LIBS += -L$$SHARED_FOLDER
}

LIBS += -ldemo_shared
