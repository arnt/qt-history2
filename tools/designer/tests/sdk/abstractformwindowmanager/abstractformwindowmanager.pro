
include(../qtestlib_sdk/qtestlib_sdk.pri)

TARGET = tst_$$TARGET

HEADERS += \
    tst_abstractformwindowmanager.h

SOURCES += \
    tst_abstractformwindowmanager.cpp \
    main.cpp

MOCABLE += \
    tst_abstractformwindowmanager.cpp
