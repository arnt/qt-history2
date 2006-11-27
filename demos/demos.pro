TEMPLATE    = subdirs
SUBDIRS     = \
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
        demos_textedit \
        demos_chip

unix:!embedded:contains(QT_CONFIG, qdbus):SUBDIRS += demos_dbus_viewer
!contains(QT_EDITION, Console):!cross_compile:!embedded:SUBDIRS += demos_arthurplugin

!cross_compile:SUBDIRS += demos_sqlbrowser

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DEMOS]
INSTALLS += sources

demos_chip.subdir = chip
demos_dbus_viewer.subdir = dbus-viewer
demos_shared.subdir = shared
demos_deform.subdir = deform
demos_gradients.subdir = gradients
demos_pathstroke.subdir = pathstroke
demos_affine.subdir = affine
demos_composition.subdir = composition
demos_books.subdir = books
demos_interview.subdir = interview
demos_mainwindow.subdir = mainwindow
demos_spreadsheet.subdir = spreadsheet
demos_textedit.subdir = textedit
demos_arthurplugin.subdir = arthurplugin
demos_sqlbrowser.subdir = sqlbrowser

#CONFIG += ordered
!ordered {
     demos_affine.depends = demos_shared
     demos_deform.depends = demos_shared
     demos_gradients.depends = demos_shared
     demos_composition.depends = demos_shared
     demos_arthurplugin.depends = demos_shared
     demos_pathstroke.depends = demos_shared
}
