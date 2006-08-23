TEMPLATE      = subdirs
unset(EXAMPLES_MAINWINDOWS_SUBDIRS)
EXAMPLES_MAINWINDOWS_SUBDIRS = examples_mainwindows_application \
                               examples_mainwindows_dockwidgets \
                               examples_mainwindows_mdi \
                               examples_mainwindows_menus \
                               examples_mainwindows_recentfiles \
                               examples_mainwindows_sdi

# install
target.path = $$[QT_INSTALL_EXAMPLES]/mainwindows
EXAMPLES_MAINWINDOWS_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS mainwindows.pro README
EXAMPLES_MAINWINDOWS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/mainwindows
INSTALLS += target EXAMPLES_MAINWINDOWS_install_sources

#subdirs
examples_mainwindows_application.subdir = $$QT_BUILD_TREE/examples/mainwindows/application
examples_mainwindows_application.depends =  src_corelib src_gui
examples_mainwindows_dockwidgets.subdir = $$QT_BUILD_TREE/examples/mainwindows/dockwidgets
examples_mainwindows_dockwidgets.depends =  src_corelib src_gui
examples_mainwindows_mdi.subdir = $$QT_BUILD_TREE/examples/mainwindows/mdi
examples_mainwindows_mdi.depends =  src_corelib src_gui
examples_mainwindows_menus.subdir = $$QT_BUILD_TREE/examples/mainwindows/menus
examples_mainwindows_menus.depends =  src_corelib src_gui
examples_mainwindows_recentfiles.subdir = $$QT_BUILD_TREE/examples/mainwindows/recentfiles
examples_mainwindows_recentfiles.depends =  src_corelib src_gui
examples_mainwindows_sdi.subdir = $$QT_BUILD_TREE/examples/mainwindows/sdi
examples_mainwindows_sdi.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_MAINWINDOWS_SUBDIRS
SUBDIRS += $$EXAMPLES_MAINWINDOWS_SUBDIRS
