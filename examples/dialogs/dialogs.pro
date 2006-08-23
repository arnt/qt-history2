TEMPLATE      = subdirs
unset(EXAMPLES_DIALOGS_SUBDIRS)
EXAMPLES_DIALOGS_SUBDIRS = examples_dialogs_configdialog \
                           examples_dialogs_extension \
                           examples_dialogs_findfiles \
                           examples_dialogs_standarddialogs \
                           examples_dialogs_tabdialog

# install
EXAMPLES_DIALOGS_install_sources.files = README *.pro
EXAMPLES_DIALOGS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/dialogs
INSTALLS += EXAMPLES_DIALOGS_install_sources

#subdirs
examples_dialogs_configdialog.subdir = $$QT_BUILD_TREE/examples/dialogs/configdialog
examples_dialogs_configdialog.depends =  src_corelib src_gui
examples_dialogs_extension.subdir = $$QT_BUILD_TREE/examples/dialogs/extension
examples_dialogs_extension.depends =  src_corelib src_gui
examples_dialogs_findfiles.subdir = $$QT_BUILD_TREE/examples/dialogs/findfiles
examples_dialogs_findfiles.depends =  src_corelib src_gui
examples_dialogs_standarddialogs.subdir = $$QT_BUILD_TREE/examples/dialogs/standarddialogs
examples_dialogs_standarddialogs.depends =  src_corelib src_gui
examples_dialogs_tabdialog.subdir = $$QT_BUILD_TREE/examples/dialogs/tabdialog
examples_dialogs_tabdialog.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_DIALOGS_SUBDIRS
SUBDIRS += $$EXAMPLES_DIALOGS_SUBDIRS
