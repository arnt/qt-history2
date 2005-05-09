INCLUDEPATH += $$SHARED_FOLDER

contains(CONFIG, debug_and_release_target) {
    debug { 
	LIBS+=-l$$SHARED_FOLDER/debug/demo_shared
    } else {
	LIBS+=-l$$SHARED_FOLDER/release/demo_shared
    }
} else {
    LIBS += -l$$SHARED_FOLDER/demo_shared
}