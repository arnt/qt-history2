TEMPLATE	= subdirs
CONFIG		+= ordered

contains( QT_PRODUCT, qt-(enterprise|free) ) {
    SUBDIRS	= src \
		  examples
} else {
    message( "QMotif requires a Qt/Enterprise license." )
}
