/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfontengine_p.h"
#include "qmemorymanager_qws.h"
#include <private/qunicodetables_p.h>
#include <qpainter.h>
#include <qgfxraster_qws.h>
#include <private/qunicodetables_p.h>
#include <qbitmap.h>
#include <qstackarray.h>
#include <private/qpainter_p.h>
#include "qpaintengine_qws.h"
#define GFX(p) static_cast<QWSPaintEngine *>(p)->gfx()

#include "qgfx_qws.h"



/*QMemoryManager::FontID*/ void *QFontEngine::handle() const
{
    return id;
}

QFontEngine::QFontEngine( const QFontDef& d, const QPaintDevice *pd )
{
    QFontDef fontDef = d;
    id = memorymanager->refFont(fontDef);
    scale = pd ? (pd->resolution()<<8)/75 : 1<<8;
}

QFontEngine::~QFontEngine()
{
    memorymanager->derefFont(id);
}


QFontEngine::FECaps QFontEngine::capabilites() const
{
    return NoTransformations;
}


/* returns 0 as glyph index for non existant glyphs */
QFontEngine::Error QFontEngine::stringToCMap( const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, bool mirrored ) const
{
    if(*nglyphs < len) {
	*nglyphs = len;
	return OutOfMemory;
    }
    if ( mirrored ) {
	for(int i = 0; i < len; i++ )
	    glyphs[i].glyph = ::mirroredChar(str[i]).unicode();
    } else {
	for(int i = 0; i < len; i++ )
	    glyphs[i].glyph = str[i].unicode();
    }
    *nglyphs = len;
    for(int i = 0; i < len; i++)
	if ( ::category(str[i]) == QChar::Mark_NonSpacing )
	    glyphs[i].advance = 0;
	else
	    glyphs[i].advance = (memorymanager->lockGlyphMetrics(handle(), str[i].unicode() )->advance*scale)>>8;
    return NoError;
}



struct QtFontEnginePos {
 	int x;
 	int y;
     };

void QFontEngine::draw( QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags )
{
#ifndef QT_NO_TRANSFORMATIONS
    if ( p->painterState()->txop > QPainter::TxScale ) {
	int aw = si.width;
	int ah = si.ascent + si.descent + 1;
	int tx = 0;
	int ty = 0;
	if ( aw == 0 || ah == 0 )
	    return;

	QWMatrix mat1 = p->painterState()->matrix;
#ifndef QT_NO_PIXMAP_TRANSFORMATION
	QWMatrix mat2 = QPixmap::trueMatrix( mat1, aw, ah );
#endif
	QPixmap *tpm = 0;
	QBitmap *wx_bm = 0;
	if ( memorymanager->fontSmooth(handle()) &&
	     QPaintDevice::qwsDisplay()->supportsDepth(32) )
	{
	    QPixmap pm(aw, ah, 32);
	    QPainter paint(&pm);
	    paint.fillRect(pm.rect(),Qt::black);
	    paint.setPen(QPen(Qt::white));
	    draw( paint.d->engine, 0, si.ascent, si, textFlags );
	    paint.end();
	    // Now we have an image with r,g,b gray scale set.
	    // Put this in alpha channel and set pixmap to pen color.
	    QRgb bg = p->painterState()->pen.color().rgb() & 0x00FFFFFF;
	    for ( int y = 0; y < ah; y++ ) {
		uint *p = (uint *)pm.scanLine(y);
		for ( int x = 0; x < aw; x++ ) {
		    int a = *p & 0xFF;
		    *p = bg | (a << 24);
		    p++;
		}
	    }
#ifndef QT_NO_PIXMAP_TRANSFORMATION
	    tpm = new QPixmap( pm.xForm( mat2 ) );
#else
	    tpm = new QPixmap( pm );
#endif
	    if ( tpm->isNull() ) {
		delete tpm;
		return;
	    }
	} else {
#if 0
	    bm_key = gen_text_bitmap_key( mat2, dfont, string, len );
	    wx_bm = get_text_bitmap( bm_key );
	    create_new_bm = wx_bm == 0;
	    if ( create_new_bm && !empty )
#endif
	    { // no such cached bitmap
		QBitmap bm( aw, ah, TRUE );	// create bitmap
		QPainter paint;
		paint.begin( &bm );		// draw text in bitmap
		paint.setPen( p->color1 );
		draw( paint.d->engine, 0, si.ascent, si, textFlags );
		paint.end();
#ifndef QT_NO_PIXMAP_TRANSFORMATION
		wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
#else
		wx_bm = new QBitmap( bm );
#endif
		if ( wx_bm->isNull() ) {
		    delete wx_bm;		// nothing to draw
		    return;
		}
	    }
	}
	double fx=x, fy=y - si.ascent, nfx, nfy;
	mat1.map( fx,fy, &nfx,&nfy );
	double tfx=tx, tfy=ty, dx, dy;
#ifndef QT_NO_PIXMAP_TRANSFORMATION
	mat2.map( tfx, tfy, &dx, &dy );     // compute position of bitmap
#endif
	x = qRound(nfx-dx);
	y = qRound(nfy-dy);

	if ( memorymanager->fontSmooth(handle()) &&
	     QPaintDevice::qwsDisplay()->supportsDepth(32) ) {
	    GFX(p)->setSource( tpm );
	    GFX(p)->setAlphaType(QGfx::InlineAlpha);
	    GFX(p)->blt(x, y, tpm->width(),tpm->height(), 0, 0);
	    delete tpm;
	    return;
	} else {
	    GFX(p)->setSource(wx_bm);
	    GFX(p)->setAlphaType(QGfx::LittleEndianMask);
	    GFX(p)->setAlphaSource(wx_bm->scanLine(0), wx_bm->bytesPerLine());
	    GFX(p)->blt(x, y, wx_bm->width(),wx_bm->height(), 0, 0);

#if 0
	    if ( create_new_bm )
		ins_text_bitmap( bm_key, wx_bm );
#else
	    delete wx_bm;
#endif
	}
	return;
    }
#endif

#ifndef QT_NO_TRANSFORMATIONS
    if (p->painterState()->txop == QPainter::TxTranslate)
#endif
	p->painterState()->painter->map( x, y, &x, &y );

    if ( textFlags ) {
	int lw = lineThickness();
	GFX(p)->setBrush( p->painterState()->pen.color() );
	if ( textFlags & Qt::Underline )
	    GFX(p)->fillRect( x, y+underlinePosition(), si.width, lw );
	if ( textFlags & Qt::StrikeOut )
	    GFX(p)->fillRect( x, y-ascent()/3, si.width, lw );
	if ( textFlags & Qt::Overline )
	    GFX(p)->fillRect( x, y-ascent()-1, si.width, lw );
	GFX(p)->setBrush( p->painterState()->brush );
    }

    QGlyphLayout *glyphs = si.glyphs;

    QStackArray<QtFontEnginePos> positions(si.num_glyphs);
    QStackArray<unsigned short> g(si.num_glyphs);

    for (int i = 0; i < si.num_glyphs; ++i)
	g[i] = glyphs[i].glyph;

    if ( si.right_to_left ) {
	int i = si.num_glyphs;
	while( i-- ) {
	    x += glyphs[i].advance;
	    glyph_metrics_t gi = boundingBox( glyphs[i].glyph );
	    positions[i].x = x-glyphs[i].offset.x-gi.xoff;
	    positions[i].y = y+glyphs[i].offset.y-gi.yoff;
	}
    } else {
	int i = 0;
	while( i < si.num_glyphs ) {
	    positions[i].x = x+glyphs[i].offset.x;
	    positions[i].y = y+glyphs[i].offset.y;
	    x += glyphs[i].advance;
	    i++;
	}
    }
//    QConstString cstr( (QChar *)g, si.num_glyphs );
    GFX(p)->drawGlyphs(handle(), g, (QPoint *)positions.data(), si.num_glyphs);
}

glyph_metrics_t QFontEngine::boundingBox( const QGlyphLayout *glyphs, int numGlyphs )
{
    if ( numGlyphs == 0 )
	return glyph_metrics_t();

    int w = 0;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while( end > glyphs )
	w += (--end)->advance;
    w = (w*scale)>>8;
    return glyph_metrics_t(0, -ascent(), w, ascent()+descent()+1, w, 0 );
}

glyph_metrics_t QFontEngine::boundingBox( glyph_t glyph )
{
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics( handle(), glyph);
    return glyph_metrics_t( (metrics->bearingx*scale)>>8, (metrics->bearingy*scale)>>8, (metrics->width*scale)>>8, (metrics->height*scale)>>8, (metrics->advance*scale)>>8, 0 );
}

bool QFontEngine::canRender( const QChar *string,  int len )
{
    bool allExist = TRUE;
    while ( len-- )
	if ( !memorymanager->inFont( handle(), string[len].unicode() ) ) {
	    allExist = FALSE;
	    break;
	}

    return allExist;
}


int QFontEngine::ascent() const
{
    return (memorymanager->fontAscent(handle())*scale)>>8;
}

int QFontEngine::descent() const
{
    return (memorymanager->fontDescent(handle())*scale)>>8;
}

int QFontEngine::leading() const
{
    return (memorymanager->fontLeading(handle())*scale)>>8;
}

int QFontEngine::maxCharWidth() const
{
    return (memorymanager->fontMaxWidth(handle())*scale)>>8;
}

int QFontEngine::minLeftBearing() const
{
    return (memorymanager->fontMinLeftBearing(handle())*scale)>>8;
}

int QFontEngine::minRightBearing() const
{
    return (memorymanager->fontMinRightBearing(handle())*scale)>>8;
}

int QFontEngine::underlinePosition() const
{
    return memorymanager->fontUnderlinePos(handle());
}

int QFontEngine::lineThickness() const
{
    return memorymanager->fontLineWidth(handle());
}

