TEMPLATE    =	subdirs

contains(DEFINES,QT_DLL) {
	SUBDIRS	+= accessible codecs imageformats sqldrivers styles
}

contains(CONFIG,dll) {
	SUBDIRS	+= accessible codecs imageformats sqldrivers styles
}
