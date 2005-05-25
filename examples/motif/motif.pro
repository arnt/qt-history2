TEMPLATE      = subdirs
SUBDIRS	     += customwidget \
                dialog

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS motif.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/motif
INSTALLS += sources
