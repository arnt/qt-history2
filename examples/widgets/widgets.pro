TEMPLATE      = subdirs
SUBDIRS       = analogclock \
                calculator \
                charactermap \
                digitalclock \
                groupbox \
                icons \
                imageviewer \
                lineedits \
                screenshot \
                scribble \
                sliders \
                spinboxes \
                styles \
                tetrix \
                tooltips \
                wiggly \
                windowflags

# install
target.path = $$[QT_INSTALL_DATA]/examples/widgets
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS widgets.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/widgets
INSTALLS += target sources
