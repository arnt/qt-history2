TEMPLATE      = subdirs
SUBDIRS       = codecs \
                i18n \
                plugandpaint \
                plugandpaintplugins \
                regexp \
                settingseditor
!cross_compile:SUBDIRS += qtdemo 

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tools.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/tools
INSTALLS += target sources
