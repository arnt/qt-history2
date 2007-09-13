TEMPLATE      = subdirs
SUBDIRS       = framebuffer mousecalibration 

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore
INSTALLS += sources
DEFINES += QT_USE_USING_NAMESPACE
