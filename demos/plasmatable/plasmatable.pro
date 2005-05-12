TEMPLATE        = app
TARGET          = plasmatable

HEADERS         = plasmamodel.h \
                  plasmadelegate.h \
                  hexdelegate.h
SOURCES         = main.cpp \
                  plasmamodel.cpp \
                  plasmadelegate.cpp \
                  hexdelegate.cpp
unix:!mac:LIBS += -lm
build_all:CONFIG += release

