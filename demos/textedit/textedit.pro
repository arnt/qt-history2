TEMPLATE        = app
TARGET          = textedit

CONFIG          += qt warn_on

QTDIR_build:REQUIRES    = "contains(QT_CONFIG, full-config)"

HEADERS         = textedit.h
SOURCES         = textedit.cpp \
                  main.cpp

RESOURCES += textedit.qrc
