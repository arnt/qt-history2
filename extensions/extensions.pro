# Project ID used by some IDEs
GUID 	 = {e7a02658-925b-4320-8f81-095bef9255b1}
TEMPLATE = subdirs

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
	win32:	SUBDIRS = activeqt
	x11:	SUBDIRS = motif
}
