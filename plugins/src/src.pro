GUID 	 = {0051e420-b700-4ec5-8c94-03facd2c8fdf}
TEMPLATE = subdirs

shared {
	SUBDIRS	*= accessible codecs imageformats sqldrivers styles
	embedded:SUBDIRS *=  gfxdrivers
}
dll {
	SUBDIRS	*= accessible codecs imageformats sqldrivers styles
	embedded:SUBDIRS *=  gfxdrivers
}
