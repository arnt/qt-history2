TEMPLATE        = app
TARGET          = textedit

CONFIG          += qt warn_on

HEADERS         = textedit.h
SOURCES         = textedit.cpp \
                  main.cpp

RESOURCES += textedit.qrc
build_all:CONFIG += release
