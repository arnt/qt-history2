TEMPLATE    =	subdirs

contains(DEFINES,QT_DLL) {
	SUBDIRS	+= accessible codecs imageformats sqldrivers styles
}
