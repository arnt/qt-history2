TEMPLATE        = app
TARGET          = textedit

CONFIG          += qt warn_on release

QTDIR_build:REQUIRES    = "contains(QT_CONFIG, full-config)"

HEADERS         = textedit.h
SOURCES         = textedit.cpp \
                  main.cpp

QMAKE_RESOURCE_PREFIX = /images
RESOURCES          = editcopy.xpm editcut.xpm editpaste.xpm editredo.xpm editundo.xpm filenew.xpm fileopen.xpm fileprint.xpm filesave.xpm textbold.xpm textcenter.xpm textitalic.xpm textjustify.xpm textleft.xpm textright.xpm textunder.xpm logo32.png
