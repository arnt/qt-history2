TEMPLATE        = app
TARGET          = textedit

CONFIG          += qt warn_on release

QTDIR_build:REQUIRES    = "contains(QT_CONFIG, full-config)"

HEADERS         = textedit.h
SOURCES         = textedit.cpp \
                  main.cpp

QMAKE_RESOURCE_PREFIX = /images
RESOURCES          = editcopy.png editcut.png editpaste.png editredo.png editundo.png filenew.png fileopen.png fileprint.png filesave.png textbold.png textcenter.png textitalic.png textjustify.png textleft.png textright.png textunder.png logo32.png
