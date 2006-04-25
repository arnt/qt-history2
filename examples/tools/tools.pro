TEMPLATE      = subdirs
CONFIG       += ordered
SUBDIRS       = codecs \
                i18n \
                plugandpaintplugins \
                plugandpaint \
                regexp \
                settingseditor
!cross_compile:SUBDIRS += qtdemo 

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tools.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/tools
INSTALLS += target sources
