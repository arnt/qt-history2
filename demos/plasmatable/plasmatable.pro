TEMPLATE        = app
TARGET          = plasmatable

HEADERS         = plasmamodel.h \
                  plasmadelegate.h \
                  colorfilter.h
SOURCES         = main.cpp \
                  plasmamodel.cpp \
                  plasmadelegate.cpp \
                  colorfilter.cpp
RESOURCES	= plasmatable.qrc

build_all:CONFIG += release

