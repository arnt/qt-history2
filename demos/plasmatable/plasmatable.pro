TEMPLATE        = app
TARGET          = plasmatable

HEADERS         = plasmamodel.h \
                  plasmadelegate.h \
                  hexdelegate.h
SOURCES         = main.cpp \
                  plasmamodel.cpp \
                  plasmadelegate.cpp \
                  hexdelegate.cpp
RESOURCES	= plasmatable.qrc
unix:x11:LIBS += -lm
build_all:CONFIG += release

