TEMPLATE = lib
CONFIG  += designer plugin debug_and_release
DESTDIR  = $$QT_BUILD_TREE/plugins/designer

CONFIG(debug, debug|release) {
    mac: TARGET = $$join(TARGET,,,_debug)
    win32: TARGET = $$join(TARGET,,d)
}

HEADERS += multipagewidget.h \
           multipagewidgetplugin.h \ 
           multipagewidgetcontainerextension.h \
           multipagewidgetextensionfactory.h 

SOURCES += multipagewidget.cpp \
           multipagewidgetplugin.cpp \
           multipagewidgetcontainerextension.cpp \
           multipagewidgetextensionfactory.cpp 

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/containerextension
INSTALLS += target sources
