TEMPLATE	= subdirs
CONFIG		+= ordered

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
    SUBDIRS	= src \
		  examples
} else {
    message( "QMotif requires a Qt/Enterprise license." )
}
