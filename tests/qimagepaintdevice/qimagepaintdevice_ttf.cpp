/****************************************************************************
**
** Implementation of QImagePaintDevice32 class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
 
#include "qpaintdevicemetrics.h"
#include "qimagepaintdevice.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qstring.h"
#include "qfont.h"
#include "qfile.h"
#include "qintcache.h"
#include <stdlib.h>
#include <freetype.h>

static char* fontdir[] = {
    "/usr/X11/lib/X11/fonts/ttfonts/",
    "./",
    0
};

#define CACHE_SIZE 200000 // bytes


bool qt_antialias_text = FALSE;

static TT_Error fterr;
#define CHKERR(f) if ((fterr=f)) qFatal("FreeType error #%ld",fterr)

class QTMapTTF : public QTMap {
public:
    QTMapTTF( TT_Engine engine, const TT_Glyph_Metrics& gmetrics,
	    const TT_Outline& outline )
    {
	TT_BBox bbox;
	CHKERR(TT_Get_Outline_BBox( (TT_Outline*)&outline, &bbox ));
	#define FLOOR(x) ((x) & -64)
	#define CEILING(x) (((x)+63) & -64)
	int xMin = FLOOR( bbox.xMin );
	int xMax = CEILING ( bbox.xMax );
	int yMin = FLOOR( bbox.yMin );
	int yMax = CEILING ( bbox.yMax );
	#undef FLOOR
	#undef CEILING
	w = (xMax - xMin) / 64;
	h = (yMax - yMin) / 64;
	off = (QPoint(xMin, -yMin))/64;
	adv = QPoint(gmetrics.advance,0); // nb. TT_Big_Glyph_Metrics
	TT_Translate_Outline( (TT_Outline*)&outline, -xMin, -yMin );

	//qDebug("  w=%d h=%d bearingX=%ld bearingY=%ld %d..%dH %d..%dV",
	//	w,h,gmetrics.bearingX,gmetrics.bearingY,xMin,xMax,yMin,yMax);
	w = (w + 3) & -4;

	TT_Raster_Map bitmap;
	bitmap.rows   = h;
	bitmap.width  = w;
	bitmap.cols   = w;
	bitmap.size   = bitmap.rows * bitmap.cols;
	bitmap.bitmap = buffer = new char[w*h];
	memset( buffer, 0, bitmap.cols*h );
	TT_Get_Outline_Pixmap( engine, (TT_Outline*)&outline, &bitmap );
	//TT_Get_Outline_Bitmap( engine, (TT_Outline*)&outline, &bitmap );
    }

    int storageCost() const
    {
	return w*h+sizeof(this)+4;
    }
};

class QFontRendererTTF : public QFontRenderer {
    static TT_Engine engine;
    static bool engine_started;
    static QIntCache<QTMapTTF> *qtmap_cache;
    TT_Face face;
    TT_Face_Properties properties;
    TT_Raster_Map raster;
    TT_CharMap charmap;
    TT_Outline outline;
    TT_Instance instance;
    TT_Glyph glyph;
    int mapid;

public:
    static QFontRenderer* renderer(const QFont& f)
    {
	if ( !engine_started ) {
	    engine_started = TRUE;
	    CHKERR(TT_Init_FreeType(&engine));
	    static TT_Byte palette[5] = { 0, 1, 2, 3, 4 }; // XXX bg(white)..fg(black)
	    CHKERR(TT_Set_Raster_Gray_Palette(engine, palette));
	}

	for (int i=0; fontdir[i]; i++) {
	    QString path = fontdir[i];
	    path += f.family();
	    path += ".ttf";

	    if ( QFile::exists(path) ) {
		return new QFontRendererTTF( (TT_Text*)path.latin1(), f.pointSizeFloat() );
	    } else if ( QFile::exists(path.lower()) ) {
		return new QFontRendererTTF( (TT_Text*)path.lower().latin1(), f.pointSizeFloat() );
	    }
	}
	return 0;
    }

    static void cleanup()
    {
	CHKERR(TT_Done_FreeType(engine));
	delete qtmap_cache;
    }

    QTMap *mapFor( QChar ch )
    {
	QTMapTTF* result;
	long key = (mapid << 16) + ch.unicode();
	if ( !qtmap_cache ) {
	    qtmap_cache = new QIntCache<QTMapTTF>( CACHE_SIZE, 3769 );
	}
	result = qtmap_cache->find( key );
	if ( !result ) {
	    TT_UShort gindex = TT_Char_Index( charmap, ch );
	    CHKERR(TT_Load_Glyph( instance, glyph, gindex, TTLOAD_DEFAULT ));
	    CHKERR(TT_Get_Glyph_Outline( glyph, &outline ));
	    TT_Glyph_Metrics gmetrics;
	    TT_Get_Glyph_Metrics( glyph, &gmetrics );

	    //qDebug("Character '%c' (U%04d)",ch.latin1(),ch.unicode());
	    result = new QTMapTTF(engine,gmetrics,outline);
	    qtmap_cache->insert(key,result,result->storageCost());
	}

	return result;
    }

    ~QFontRendererTTF()
    {
	CHKERR(TT_Done_Outline(&outline));
	CHKERR(TT_Close_Face(face));
    }

private:
    static int nextid;
    QFontRendererTTF(TT_Text* fontPathName, double pointSize)
    {
	mapid = nextid++;
	CHKERR(TT_Open_Face( engine, fontPathName, &face ));
	CHKERR(TT_Get_Face_Properties( face, &properties ));
	CHKERR(TT_New_Instance( face, &instance ));
	CHKERR(TT_Set_Instance_Resolutions( instance, 75, 75 ));
	CHKERR(TT_Set_Instance_CharSize( instance,
	    int(pointSize*64) ));
	//TT_Instance_Metrics imetrics;
	//TT_Get_Instance_Metrics( instance, &imetrics );
	CHKERR(TT_New_Outline( properties.max_Points, properties.max_Contours,
	    &outline ));

	CHKERR(TT_New_Glyph( face, &glyph ));
	int bestmap = 0;
	TT_UShort pid, eid;
	int i;

	// This way...
	TT_UShort lid, nid;
	int nn = properties.num_Names;
	for (i=0; i<nn; i++) {
	    CHKERR(TT_Get_Name_ID( face, i, &pid, &eid, &lid, &nid ));
	    TT_String* name;
	    TT_UShort len;
	    CHKERR(TT_Get_Name_String( face, nid, &name, &len ));
	    QString s;
	    for (int c=0; c<len; c++)
		s += name[c];
	    qDebug("NAME=%s LID=%d PID=%d  EID=%d",s.latin1(),lid,pid,eid);
	}

	// or this way...
	int n = properties.num_CharMaps;
	for (i=0; i<n; i++) {
	    CHKERR(TT_Get_CharMap_ID( face, i, &pid, &eid ));
	    qDebug("PID=%d  EID=%d",pid,eid);
	}

	CHKERR(TT_Get_CharMap( face, bestmap, &charmap ));
    }
};

TT_Engine QFontRendererTTF::engine;
bool QFontRendererTTF::engine_started=FALSE;
int QFontRendererTTF::nextid=1;
QIntCache<QTMapTTF> *QFontRendererTTF::qtmap_cache=0;

QFontRenderer* qt_font_renderer_ttf(const QFont& f)
{
    return QFontRendererTTF::renderer(f);
}

