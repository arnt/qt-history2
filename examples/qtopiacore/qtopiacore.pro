TEMPLATE      = subdirs
SUBDIRS       = mousecalibration \
                svgalib

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore
INSTALLS += sources
