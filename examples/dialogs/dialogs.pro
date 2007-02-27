TEMPLATE      = subdirs
SUBDIRS       = classwizard \
                configdialog \
                extension \
                findfiles \
                licensewizard \
                standarddialogs \
                tabdialog \
                trivialwizard

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/dialogs
INSTALLS += sources
