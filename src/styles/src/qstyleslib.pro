TEMPLATE	= subdirs
SUBDIRS		= motif \
		  motifplus \
		  cde \
		  sgi \
	 	  windows \
		  platinum \
		  compact 
mac:SUBDIRS   += aqua
win32:SUBDIRS  += windowsxp
