GUID 	 = {5bfc0ba6-7a62-4042-94e2-d6cec2333474}
TEMPLATE = subdirs

CONFIG	+= ordered

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
} else {

	SUBDIRS	= idc \
		  testcon \
		  dumpdoc
}
