TEMPLATE    = subdirs
DEMOS_SUBDIRS     = \
	demos_shared \
	demos_deform \
	demos_gradients \
	demos_pathstroke \
	demos_affine \
	demos_composition \
        demos_books \
        demos_interview \
        demos_mainwindow \
        demos_spreadsheet \
        demos_textedit 

unix:!embedded:contains(QT_CONFIG, qdbus):DEMOS_SUBDIRS += demos_dbus-viewer
!contains(QT_EDITION, Console):!cross_compile:DEMOS_SUBDIRS += demos_arthurplugin

!cross_compile:DEMOS_SUBDIRS += demos_sqlbrowser

# install
DEMOS_install_sources.files = README *.pro
DEMOS_install_sources.path = $$[QT_INSTALL_DEMOS]
INSTALLS += DEMOS_install_sources

# This creates a sub-demos rule
sub_demos_target.CONFIG = recursive
sub_demos_target.recurse = $$DEMOS_SUBDIRS $$DEMOS_SUB_SUBDIRS 
sub_demos_target.target = sub-demos
sub_demos_target.recurse_target =
QMAKE_EXTRA_TARGETS += sub_demos_target

demos_shared.subdir = $$QT_BUILD_TREE/demos/shared
demos_shared.depends = src_corelib src_gui
demos_dbus-viewer.subdir = $$QT_BUILD_TREE/demos/dbus-viewer
demos_dbus-viewer.depends = src_corelib src_gui tools_qdbus_src
demos_deform.subdir = $$QT_BUILD_TREE/demos/deform
demos_deform.depends = demos_shared
demos_gradients.subdir = $$QT_BUILD_TREE/demos/gradients
demos_gradients.depends = demos_shared
demos_pathstroke.subdir = $$QT_BUILD_TREE/demos/pathstroke
demos_pathstroke.depends = demos_shared
demos_affine.subdir = $$QT_BUILD_TREE/demos/affine
demos_affine.depends = demos_shared
demos_composition.subdir = $$QT_BUILD_TREE/demos/composition
demos_composition.depends = demos_shared
demos_books.subdir = $$QT_BUILD_TREE/demos/books
demos_books.depends = src_sql src_gui
demos_interview.subdir = $$QT_BUILD_TREE/demos/interview
demos_interview.depends =  src_corelib src_gui
demos_mainwindow.subdir = $$QT_BUILD_TREE/demos/mainwindow
demos_mainwindow.depends =  src_corelib src_gui
demos_spreadsheet.subdir = $$QT_BUILD_TREE/demos/spreadsheet
demos_spreadsheet.depends =  src_corelib src_gui
demos_textedit.subdir = $$QT_BUILD_TREE/demos/textedit
demos_textedit.depends =  src_corelib src_gui
demos_arthurplugin.subdir = $$QT_BUILD_TREE/demos/arthurplugin
demos_arthurplugin.depends = demos_shared tools_designer_src_lib
demos_sqlbrowser.subdir = $$QT_BUILD_TREE/demos/sqlbrowser
demos_sqlbrowser.depends = src_sql src_gui

SUBDIRS += $$DEMOS_SUBDIRS
