TEMPLATE = subdirs

dll|shared {
	SUBDIRS	*= accessible codecs imageformats sqldrivers styles
	embedded:SUBDIRS *=  gfxdrivers
}
