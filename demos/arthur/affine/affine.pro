SOURCES += main.cpp xform.cpp
HEADERS += xform.h

SHARED_FOLDER = ../shared

include($$SHARED_FOLDER/shared.pri)

RESOURCES += affine.qrc
