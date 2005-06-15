INCLUDEPATH += $$QT_BUILD_TREE/util/licensekeys/shared

HEADERS += $$QT_BUILD_TREE/util/licensekeys/shared/keyinfo.h

SOURCES += \
	$$QT_BUILD_TREE/src/corelib/kernel/qtcore_eval.cpp \
	$$QT_BUILD_TREE/util/licensekeys/shared/keyinfo.cpp

DEFINES += QT_EVAL
