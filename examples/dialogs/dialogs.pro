TEMPLATE      = subdirs
SUBDIRS       = complexwizard \
                configdialog \
                extension \
                findfiles \
                simplewizard \
                standarddialogs \
                tabdialog

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/dialogs
INSTALLS += sources
