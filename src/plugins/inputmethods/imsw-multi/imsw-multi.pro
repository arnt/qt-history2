TEMPLATE = lib
TARGET   = qimsw-multi

CONFIG      += qt plugin
DESTDIR = $$QT_BUILD_TREE/plugins/inputmethods
#INCLUDEPATH += .

HEADERS += qmultiinputcontext.h \
           qmultiinputcontextplugin.h
SOURCES += qmultiinputcontext.cpp \
           qmultiinputcontextplugin.cpp

target.path += $$plugins.path/inputmethods
INSTALLS    += target
