TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../echowindow
HEADERS         = echoplugin.h
SOURCES         = echoplugin.cpp
TARGET          = echoplugin

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin/plugin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugin.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin/plugin
INSTALLS += target sources
