# Project ID used by some IDEs
GUID 	 = {31e51ade-a61b-4b36-bf27-9bc3f5ea95f6}
TEMPLATE = subdirs

CONFIG	+= ordered

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
    SUBDIRS	= src \
		  examples
} else {
    message( "QMotif requires a Qt/Enterprise license." )
}
