TEMPLATE	= subdirs
CONFIG		+= ordered

!contains( QT_PRODUCT, qt-enterprise ) {
    message( "QMotif requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-enterprise ) {
    SUBDIRS	= src \
		  examples
}
