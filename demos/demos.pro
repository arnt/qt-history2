TEMPLATE    = subdirs
SUBDIRS     = \
	shared \
	deform \
	gradients \
	pathstroke \
	affine \
        interview \
        mainwindow \
        spreadsheet \
        textedit

CONFIG += ordered

!cross_compile:SUBDIRS += sqlbrowser