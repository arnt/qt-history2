TEMPLATE      = subdirs
SUBDIRS       = codecs \
                i18n \
                plugandpaint \
                plugandpaintplugins \
                qtdemo \
                regexp \
                settingseditor

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tools.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/tools
INSTALLS += target sources
