# Project ID used by some IDEs
GUID 	 = {557d8dbf-98ed-43a6-aacf-8a7949a95186}
TEMPLATE = subdirs

CONFIG	+= ordered

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {

	SUBDIRS	= container \
		  control \
		  tools \
		  examples

	shared:SUBDIRDS += plugin
}
