TEMPLATE	= subdirs
SUBDIRS		= motif \
		  motifplus \
		  cde \
		  sgi \
	 	  windows \
		  platinum \
		  compact 
macx:SUBDIRS   += aqua
win32:SUBDIRS  += windowsxp
