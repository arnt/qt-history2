TEMPLATE    =	subdirs

contains(DEFINES,QT_DLL) {
	SUBDIRS	+=   imageformats accessible codecs
}
