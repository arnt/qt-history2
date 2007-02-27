TEMPLATE        = app
TARGET          = qdbusviewer

HEADERS         = qdbusviewer.h \
                  qdbusmodel.h \
                  propertydialog.h

SOURCES         = qdbusviewer.cpp \
                  qdbusmodel.cpp \
                  propertydialog.cpp \
                  main.cpp

DESTDIR = ../../../../bin

CONFIG += qdbus
QT += xml

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

