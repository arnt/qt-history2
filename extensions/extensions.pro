TEMPLATE = subdirs

contains( QT_PRODUCT, qt-enterprise ) {
	win32:	SUBDIRS = activeqt
	x11:	SUBDIRS = motif
}
