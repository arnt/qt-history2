
DEFINES += _STANDALONE_ QT_RASTER_PAINTENGINE

RASTERDIR = ../../../../research/painter/pixelbuffer

SOURCES += 					\
 	$$RASTERDIR/qpaintengine_raster.cpp	\
	$$RASTERDIR/qdrawhelper.cpp 		\
	$$RASTERDIR/ftraster.c			\
	$$RASTERDIR/ftgrays.c

HEADERS += 					\
	$$RASTERDIR/qpaintengine_raster_p.h 	\
	$$RASTERDIR/ftraster.h			\
	$$RASTERDIR/ftgrays.h			\
	$$RASTERDIR/ftimage.h

INCLUDEPATH += $$RASTERDIR