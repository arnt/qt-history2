TEMPLATE	= subdirs
CONFIG		+= ordered

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "QtService requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    SUBDIRS	= src \
		  examples
}
