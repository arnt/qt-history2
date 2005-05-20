TEMPLATE      = subdirs
SUBDIRS	     += customwidget \
                dialog

# install
target.path = $$[QT_INSTALL_DATA]/examples/motif
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS motif.pro
sources.path = $$[QT_INSTALL_DATA]/examples/motif
INSTALLS += target sources
