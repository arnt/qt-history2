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

#include "qbitmap.h"

// #define FONTENGINE_DEBUG

#include <qbytearray.h>
#include <qtextcodec.h>

#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qstackarray.h"

#include "qpaintengine_x11.h"
#include <private/qpaintengine_x11_p.h>
#include <private/qpainter_p.h>

#include <private/qt_x11_p.h>

#include "qfont.h"
#include "qtextengine_p.h"
#include "qfontengine_p.h"

#include <private/qunicodetables_p.h>

#include <limits.h>

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding( const char *encoding );
extern int qt_xlfd_encoding_id( const char *encoding );

extern void qt_draw_transformed_rect( QPaintEngine *p, int x, int y, int w, int h, bool fill );

static void drawLines( QPaintEngine *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags )
{
    int lw = fe->lineThickness();
    if ( textFlags & Qt::Underline ) {
    	int pos = fe->underlinePosition();
	qt_draw_transformed_rect( p, x1, baseline+pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::Overline ) {
	int pos = fe->ascent()+1;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::StrikeOut ) {
	int pos = fe->ascent()/3;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
}

QFontEngine::~QFontEngine()
{
}

int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

int QFontEngine::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = sizeof( QFontEngineBox );
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::FECaps QFontEngineBox::capabilites() const
{
    return FullTransformations;
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *, int len, QGlyphLayout *glyphs, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    memset(glyphs, 0, len * sizeof(glyph_t));

    for ( int i = 0; i < len; i++ )
	(glyphs++)->advance = _size;

    *nglyphs = len;
    return NoError;
}

void QFontEngineBox::draw( QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags )
{
    Display *dpy = QX11Info::appDisplay();
    Qt::HANDLE hd = p->handle();
    GC gc = static_cast<QX11PaintEngine *>(p)->d->gc;

    if ( p->painterState()->txop > QPainter::TxTranslate ) {
	int xp = x;
	int yp = _size + 2;
	int s = _size - 3;
	for (int k = 0; k < si.num_glyphs; k++) {
	    qt_draw_transformed_rect( p, xp, yp, s, s, FALSE );
	    xp += _size;
	}
    } else {
	if ( p->painterState()->txop == QPainter::TxTranslate )
	    p->painterState()->painter->map( x, y, &x, &y );
	XRectangle _rects[32];
	XRectangle *rects = _rects;
	if ( si.num_glyphs > 32 )
	    rects = new XRectangle[si.num_glyphs];
	for (int k = 0; k < si.num_glyphs; k++) {
	    rects[k].x = x + (k * _size);
	    rects[k].y = y - _size + 2;
	    rects[k].width = rects[k].height = _size - 3;
	}

	XDrawRectangles(dpy, hd, gc, rects, si.num_glyphs);
	if ( rects != _rects )
	    delete [] rects;
    }

    if ( textFlags != 0 )
	drawLines( p, this, y, x, si.num_glyphs*_size, textFlags );
}

glyph_metrics_t QFontEngineBox::boundingBox( const QGlyphLayout *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox( glyph_t )
{
    return glyph_metrics_t( 0, _size, _size, _size, _size, 0 );
}



int QFontEngineBox::ascent() const
{
    return _size;
}

int QFontEngineBox::descent() const
{
    return 0;
}

int QFontEngineBox::leading() const
{
    int l = qRound( _size * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

int QFontEngineBox::cmap() const
{
    return -1;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *, int )
{
    return TRUE;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}




// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------

static inline XCharStruct *charStruct( XFontStruct *xfs, uint ch )
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if ( r >= xfs->min_byte1 &&
	 r <= xfs->max_byte1 &&
	 c >= xfs->min_char_or_byte2 &&
	 c <= xfs->max_char_or_byte2) {
	if ( !xfs->per_char )
	    xcs = &(xfs->min_bounds);
	else {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
		xcs = 0;
	}
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD( XFontStruct *fs, const char *name, int mib )
    : _fs( fs ), _name( name ), _codec( 0 ), _scale( 1. ), _cmap( mib )
{
    if ( _cmap ) _codec = QTextCodec::codecForMib( _cmap );

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
		   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
		  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
		  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QX11Info::appDisplay(), _fs );
    _fs = 0;
}

QFontEngine::FECaps QFontEngineXLFD::capabilites() const
{
    return NoTransformations;
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	bool haveNbsp = FALSE;
	for ( int i = 0; i < len; i++ )
	    if ( str[i].unicode() == 0xa0 ) {
		haveNbsp = TRUE;
		break;
	    }

	QStackArray<unsigned short> ch(len);
	QChar *chars = (QChar *)ch.data();
	if ( haveNbsp || mirrored ) {
	    for ( int i = 0; i < len; i++ )
		chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
			    (mirrored ? ::mirroredChar(str[i]).unicode() : str[i].unicode()));
	} else {
	    for ( int i = 0; i < len; i++ )
		chars[i] = (str[i].unicode() == 0xa0 ? 0x20 : str[i].unicode());
	}
	QStackArray<glyph_t> g(len);
	_codec->fromUnicode( chars, g, len );
	for ( int i = 0; i < len; i++ )
	    glyphs[i].glyph = g[i];
    } else {
	QGlyphLayout *g = glyphs + len;
	const QChar *c = str + len;
	if ( mirrored ) {
	    while ( c != str )
		(--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : ::mirroredChar(*c).unicode();
	} else {
	    while ( c != str )
		(--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
	}
    }
    *nglyphs = len;

    QGlyphLayout *g = glyphs + len;
    XCharStruct *xcs;
    // inlined for better perfomance
    if ( !_fs->per_char ) {
	xcs = &_fs->min_bounds;
	while ( g != glyphs )
	    (--g)->advance = xcs->width;
    }
    else if ( !_fs->max_byte1 ) {
	XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
	while ( g != glyphs ) {
	    unsigned int gl = (--g)->glyph;
	    xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
		  base + gl : 0;
	    g->advance = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
	}
    }
    else {
	while ( g != glyphs ) {
	    xcs = charStruct( _fs, (--g)->glyph );
	    g->advance = (xcs ? xcs->width : _fs->ascent);
	}
    }
    if ( _scale != 1. ) {
	for ( int i = 0; i < len; i++ )
	    glyphs[i].advance = qRound(glyphs[i].advance*_scale);
    }
    return NoError;
}

void QFontEngineXLFD::draw( QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags )
{
    if ( !si.num_glyphs )
	return;

    // since we advocate we can't do translations this should hold.
    Q_ASSERT(p->painterState()->txop <= QPainter::TxTranslate);

//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, si.num_glyphs );

    Display *dpy = QX11Info::appDisplay();
    Qt::HANDLE hd = p->handle();
    GC gc = static_cast<QX11PaintEngine *>(p)->d->gc;

    int xorig = x;
    int yorig = y;

    Qt::HANDLE font_id = _fs->fid;
    if ( p->painterState()->txop == QPainter::TxTranslate )
	p->painterState()->painter->map( x, y, &x, &y );

    XSetFont(dpy, gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, si.num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    QGlyphLayout *glyphs = si.glyphs;

    QStackArray<XChar2b> chars(si.num_glyphs);

    for (int i = 0; i < si.num_glyphs; i++) {
	chars[i].byte1 = glyphs[i].glyph >> 8;
	chars[i].byte2 = glyphs[i].glyph & 0xff;
    }

    int xpos = x;

    if ( si.right_to_left ) {
	int i = si.num_glyphs;
	while( i-- ) {
	    advance_t adv = glyphs[i].advance;
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i].glyph );
	    int xp = x-glyphs[i].offset.x-gi.xoff;
	    int yp = y+glyphs[i].offset.y-gi.yoff;
	    XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
	}
    } else {
	if (si.hasPositioning) {
	    int i = 0;
	    while( i < si.num_glyphs ) {
		int xp = x+glyphs[i].offset.x;
		int yp = y+glyphs[i].offset.y;
		XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
		advance_t adv = glyphs[i].advance;
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += adv;
		i++;
	    }
	} else {
	    // we can take a shortcut
	    XDrawString16(dpy, hd, gc, x, y, chars, si.num_glyphs );
	    x += si.width;
	}
    }

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, x-xpos, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < si.num_glyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i].glyph );
	p->drawRect( x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y,
	       glyphs[i].advance.x, glyphs[i].advance.y);
	x += glyphs[i].advance.x;
	y += glyphs[i].advance.y;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineXLFD::boundingBox( const QGlyphLayout *glyphs, int numGlyphs )
{
    int i;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i].glyph );
	if (xcs) {
	    int x = overall.xoff + glyphs[i].offset.x - xcs->lbearing;
	    int y = overall.yoff + glyphs[i].offset.y - xcs->ascent;
	    overall.x = qMin( overall.x, x );
	    overall.y = qMin( overall.y, y );
	    xmax = qMax( xmax, overall.xoff + glyphs[i].offset.x + xcs->rbearing );
	    ymax = qMax( ymax, y + xcs->ascent + xcs->descent );
	    overall.xoff += glyphs[i].advance;
	} else {
	    int size = ascent();
	    overall.x = qMin(overall.x, overall.xoff );
	    overall.y = qMin(overall.y, overall.yoff - size );
	    ymax = qMax( ymax, overall.yoff );
	    overall.xoff += size;
	    xmax = qMax( xmax, overall.xoff );
	}
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.height = qRound(overall.height * _scale);
	overall.width = qRound(overall.width * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
    }
    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox( glyph_t glyph )
{
    glyph_metrics_t gm;
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	gm = glyph_metrics_t( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    } else {
	int size = ascent();
	gm = glyph_metrics_t( 0, size, size, size, size, 0 );
    }
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}


int QFontEngineXLFD::ascent() const
{
    return qRound(_fs->ascent*_scale);
}

int QFontEngineXLFD::descent() const
{
    return qRound((_fs->descent-1)*_scale);
}

int QFontEngineXLFD::leading() const
{
    int l = qRound((qMin(_fs->ascent, _fs->max_bounds.ascent)
		    + qMin(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXLFD::maxCharWidth() const
{
    return qRound(_fs->max_bounds.width*_scale);
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
	     (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
	    f->max_char_or_byte2 - f->min_char_or_byte2);
}

int QFontEngineXLFD::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->lbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].lbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->lbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return qRound (lbearing*_scale);
}

int QFontEngineXLFD::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->rbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].rbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->rbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return qRound (rbearing*_scale);
}

int QFontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender( const QChar *string, int len )
{
    QStackArray<QGlyphLayout, 256> glyphs(len);
    int nglyphs = len;
    if ( stringToCMap( string, len, glyphs, &nglyphs, FALSE ) == OutOfMemory ) {
	glyphs.resize(nglyphs);
	stringToCMap( string, len, glyphs, &nglyphs, FALSE );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !glyphs[i].glyph || !charStruct( _fs, glyphs[i].glyph ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    return allExist;
}


void QFontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


QFontEngine::Type QFontEngineXLFD::type() const
{
    return XLFD;
}


// ------------------------------------------------------------------
// LatinXLFD engine
// ------------------------------------------------------------------

static const int engine_array_inc = 4;

QFontEngineLatinXLFD::QFontEngineLatinXLFD( XFontStruct *xfs, const char *name,
					    int mib )
{
    _engines = new QFontEngine*[ engine_array_inc ];
    _engines[0] = new QFontEngineXLFD( xfs, name, mib );
    _count = 1;

    cache_cost = _engines[0]->cache_cost;

    memset( glyphIndices, 0, sizeof( glyphIndices ) );
    memset( glyphAdvances, 0, sizeof( glyphAdvances ) );
    euroIndex = 0;
    euroAdvance = 0;
}

QFontEngineLatinXLFD::~QFontEngineLatinXLFD()
{
    for ( int i = 0; i < _count; ++i ) {
	delete _engines[i];
	_engines[i] = 0;
    }
    delete [] _engines;
    _engines = 0;
}

QFontEngine::FECaps QFontEngineLatinXLFD::capabilites() const
{
    return NoTransformations;
}

void QFontEngineLatinXLFD::findEngine( const QChar &ch )
{
    if ( ch.unicode() == 0 ) return;

    static const char *alternate_encodings[] = {
	"iso8859-1",
	"iso8859-2",
	"iso8859-3",
	"iso8859-4",
	"iso8859-9",
	"iso8859-10",
	"iso8859-13",
	"iso8859-14",
	"iso8859-15",
	"hp-roman8"
    };
    static const int mib_count = sizeof( alternate_encodings ) / sizeof( const char * );

    // see if one of the above mibs can map the char we want
    QTextCodec *codec = 0;
    int which = -1;
    int i;
    for ( i = 0; i < mib_count; ++i ) {
	const int mib = qt_mib_for_xlfd_encoding( alternate_encodings[i] );
	bool skip = FALSE;
	for ( int e = 0; e < _count; ++e ) {
	    if ( _engines[e]->cmap() == mib ) {
		skip = TRUE;
		break;
	    }
	}
	if ( skip ) continue;

	codec = QTextCodec::codecForMib( mib );
	if ( codec && codec->canEncode( ch ) ) {
	    which = i;
	    break;
	}
    }

    if ( ! codec || which == -1 )
	return;

    const int enc_id = qt_xlfd_encoding_id( alternate_encodings[which] );
    QFontDef req = fontDef;
    QFontEngine *engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
    if ( ! engine ) {
	req.family = QString::null;
	engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
	if ( ! engine ) return;
    }
    engine->setScale( scale() );

    if ( ! ( _count % engine_array_inc ) ) {
	// grow the engines array
	QFontEngine **old = _engines;
	int new_size =
	    ( ( ( _count+engine_array_inc ) / engine_array_inc ) * engine_array_inc );
	_engines = new QFontEngine*[new_size];
	for ( i = 0; i < _count; ++i )
	    _engines[i] = old[i];
	delete [] old;
    }

    _engines[_count] = engine;
    const int hi = _count << 8;
    ++_count;

    unsigned short chars[0x201];
    QGlyphLayout glyphs[0x201];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    engine->stringToCMap( (const QChar *) chars, 0x201, glyphs, &glyphCount, FALSE );

    // merge member data with the above
    for ( i = 0; i < 0x200; ++i ) {
	if ( glyphIndices[i] != 0 || glyphs[i].glyph == 0 ) continue;
	glyphIndices[i] = hi | glyphs[i].glyph;
	glyphs[i].advance = glyphs[i].advance;
    }
    if (!euroIndex && glyphs[0x200].glyph) {
	euroIndex = hi | glyphs[0x200].glyph;
	euroAdvance = glyphs[0x200].advance;
    }
}

QFontEngine::Error
QFontEngineLatinXLFD::stringToCMap( const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    int i;
    bool missing = FALSE;
    const QChar *c = str+len;
    QGlyphLayout *g = glyphs+len;
    int asc = ascent();
    if ( mirrored ) {
	while ( c != str ) {
	    --c;
	    --g;
	    if ( c->unicode() < 0x200 ) {
		unsigned short ch = ::mirroredChar(*c).unicode();
		g->glyph = glyphIndices[ch];
		g->advance = glyphAdvances[ch];
	    } else {
		if ( c->unicode() == 0x20ac ) {
		    g->glyph = euroIndex;
		    g->advance = euroAdvance;
		} else {
		    g->glyph = 0;
		    g->advance = asc;
		}
	    }
	    missing = ( missing || ( g->glyph == 0 ) );
	}
    } else {
	while ( c != str ) {
	    --c;
	    --g;
	    if ( c->unicode() < 0x200 ) {
		g->glyph = glyphIndices[c->unicode()];
		g->advance = glyphAdvances[c->unicode()];
	    } else {
		if ( c->unicode() == 0x20ac ) {
		    g->glyph = euroIndex;
		    g->advance = euroAdvance;
		} else {
		    g->glyph = 0;
		    g->advance = asc;
		}
	    }
	    missing = ( missing || ( g->glyph == 0 ) );
	}
    }

    if ( missing ) {
	for ( i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    if ( glyphs[i].glyph != 0 || (uc >= 0x200 && uc != 0x20ac) )
		continue;

	    QFontEngineLatinXLFD *that = (QFontEngineLatinXLFD *) this;
	    that->findEngine( str[i] );
	    glyphs[i].glyph = (uc == 0x20ac ? euroIndex : that->glyphIndices[uc]);
	    glyphs[i].advance = (uc == 0x20ac ? euroAdvance : glyphAdvances[uc]);
	}
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineLatinXLFD::draw( QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags )
{
    if ( !si.num_glyphs ) return;

    QGlyphLayout *glyphs = si.glyphs;
    int which = glyphs[0].glyph >> 8;

    int start = 0;
    int end, i;
    for ( end = 0; end < si.num_glyphs; ++end ) {
	const int e = glyphs[end].glyph >> 8;
	if ( e == which ) continue;

	// set the high byte to zero
	for ( i = start; i < end; ++i )
	    glyphs[i].glyph = glyphs[i].glyph & 0xff;

	// draw the text
	QTextItem si2 = si;
	si2.glyphs = si.glyphs + start;
	si2.num_glyphs = end - start;
	_engines[which]->draw( p, x, y, si2, textFlags );

	// reset the high byte for all glyphs and advance to the next sub-string
	const int hi = which << 8;
	for ( i = start; i < end; ++i ) {
	    glyphs[i].glyph = hi | glyphs[i].glyph;
	    x += glyphs[i].advance;
	}

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    for ( i = start; i < end; ++i )
	glyphs[i].glyph = glyphs[i].glyph & 0xff;

    // draw the text
    QTextItem si2 = si;
    si2.glyphs = si.glyphs + start;
    si2.num_glyphs = end - start;
    _engines[which]->draw( p, x, y, si2, textFlags );

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for ( i = start; i < end; ++i )
	glyphs[i].glyph = hi | glyphs[i].glyph;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( const QGlyphLayout *glyphs_const, int numGlyphs )
{
    if ( numGlyphs <= 0 ) return glyph_metrics_t();

    glyph_metrics_t overall;

    QGlyphLayout *glyphs = const_cast<QGlyphLayout *>(glyphs_const);
    int which = glyphs[0].glyph >> 8;

    int start = 0;
    int end, i;
    for ( end = 0; end < numGlyphs; ++end ) {
	const int e = glyphs[end].glyph >> 8;
	if ( e == which ) continue;

	// set the high byte to zero
	for ( i = start; i < end; ++i )
	    glyphs[i].glyph = glyphs[i].glyph & 0xff;

	// merge the bounding box for this run
	const glyph_metrics_t gm =
	    _engines[which]->boundingBox( glyphs + start, end - start );

	overall.x = qMin( overall.x, gm.x );
	overall.y = qMin( overall.y, gm.y );
	overall.width = overall.xoff + gm.width;
	overall.height = qMax( overall.height + overall.y, gm.height + gm.y ) -
			 qMin( overall.y, gm.y );
	overall.xoff += gm.xoff;
	overall.yoff += gm.yoff;

	// reset the high byte for all glyphs
	const int hi = which << 8;
	for ( i = start; i < end; ++i )
	    glyphs[i].glyph = hi | glyphs[i].glyph;

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    for ( i = start; i < end; ++i )
	glyphs[i].glyph = glyphs[i].glyph & 0xff;

    // merge the bounding box for this run
    const glyph_metrics_t gm = _engines[which]->boundingBox( glyphs + start, end - start );

    overall.x = qMin( overall.x, gm.x );
    overall.y = qMin( overall.y, gm.y );
    overall.width = overall.xoff + gm.width;
    overall.height = qMax( overall.height + overall.y, gm.height + gm.y ) -
		     qMin( overall.y, gm.y );
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for ( i = start; i < end; ++i )
	glyphs[i].glyph = hi | glyphs[i].glyph;

    return overall;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( glyph_t glyph )
{
    const int engine = glyph >> 8;
    Q_ASSERT( engine < _count );
    return _engines[engine]->boundingBox( glyph & 0xff );
}

int QFontEngineLatinXLFD::ascent() const
{
    return _engines[0]->ascent();
}

int QFontEngineLatinXLFD::descent() const
{
    return _engines[0]->descent();
}

int QFontEngineLatinXLFD::leading() const
{
    return _engines[0]->leading();
}

int QFontEngineLatinXLFD::maxCharWidth() const
{
    return _engines[0]->maxCharWidth();
}

int QFontEngineLatinXLFD::minLeftBearing() const
{
    return _engines[0]->minLeftBearing();
}

int QFontEngineLatinXLFD::minRightBearing() const
{
    return _engines[0]->minRightBearing();
}

const char *QFontEngineLatinXLFD::name() const
{
    return _engines[0]->name();
}

bool QFontEngineLatinXLFD::canRender( const QChar *string, int len )
{
    bool all = TRUE;
    int i;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ||
	     glyphIndices[string[i].unicode()] == 0 ) {
	    if (string[i].unicode() != 0x20ac || euroIndex == 0)
		all = FALSE;
	    break;
	}
    }

    if ( all )
	return TRUE;

    all = TRUE;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ) {
	    if (string[i].unicode() == 0x20ac) {
		if (euroIndex)
		    continue;

		findEngine(string[i]);
		if (euroIndex)
		    continue;
	    }
	    all = FALSE;
	    break;
	}
	if ( glyphIndices[string[i].unicode()] != 0 ) continue;

	findEngine( string[i] );
	if ( glyphIndices[string[i].unicode()] == 0 ) {
	    all = FALSE;
	    break;
	}
    }

    return all;
}

void QFontEngineLatinXLFD::setScale( double scale )
{
    int i;
    for ( i = 0; i < _count; ++i )
	_engines[i]->setScale( scale );
    unsigned short chars[0x201];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    QGlyphLayout glyphs[0x201];
    _engines[0]->stringToCMap( (const QChar *)chars, 0x200,
			       glyphs, &glyphCount, FALSE );
    for (int i = 0; i < 0x200; ++i) {
	glyphIndices[i] = glyphs[i].glyph;
	glyphAdvances[i] = glyphs[i].advance;
    }
    euroIndex = glyphs[0x200].glyph;
    euroAdvance = glyphs[0x200].advance;
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------
// #define FONTENGINE_DEBUG

#ifndef QT_NO_XFTFREETYPE
class Q_HackPaintDevice : public QPaintDevice
{
public:
    inline Q_HackPaintDevice() : QPaintDevice( 0 ) {}
    inline XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }

};

static inline void getGlyphInfo( XGlyphInfo *xgi, XftFont *font, int glyph )
{
    FT_UInt x = glyph;
    XftGlyphExtents( QX11Info::appDisplay(), font, &x, 1, xgi );
}

static inline FT_Face lockFTFace( XftFont *font )
{
    return XftLockFace( font );
}

static inline void unlockFTFace( XftFont *font )
{
    XftUnlockFace( font );
}



QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap ), transformed_fonts(0)
{
    _face = lockFTFace( _font );

    cache_cost = _font->height * _font->max_advance_width *
		 ( _face ? _face->num_glyphs : 1024 );

    // if the Xft font is not antialiased, it uses bitmaps instead of
    // 8-bit alpha maps... adjust the cache_cost to reflect this
    Bool antialiased = TRUE;
    if ( XftPatternGetBool( pattern, XFT_ANTIALIAS,
			    0, &antialiased ) == XftResultMatch &&
	 ! antialiased ) {
	cache_cost /= 8;
    }
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    memset( widthCache, 0, sizeof(widthCache) );
    memset( cmapCache, 0, sizeof(cmapCache) );
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    unlockFTFace( _font );

    XftFontClose( QX11Info::appDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XftFontClose( QX11Info::appDisplay(), trf->xft_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

QFontEngine::FECaps QFontEngineXft::capabilites() const
{
    return (_face->face_flags & FT_FACE_FLAG_SCALABLE) ? FullTransformations : NoTransformations;
}

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( mirrored ) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = ::mirroredChar(str[i]).unicode();
	    glyphs[i].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i].glyph ) {
		glyph_t glyph = XftCharIndex( QX11Info::appDisplay(), _font, uc );
		glyphs[i].glyph = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    } else {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    glyphs[i].glyph = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i].glyph ) {
		glyph_t glyph = XftCharIndex( QX11Info::appDisplay(), _font, uc );
		glyphs[i].glyph = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    }

    for ( int i = 0; i < len; i++ ) {
	FT_UInt glyph = glyphs[i].glyph;
	glyphs[i].advance = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !glyphs[i].advance ) {
	    XGlyphInfo gi;
	    XftGlyphExtents( QX11Info::appDisplay(), _font, &glyph, 1, &gi );
	    glyphs[i].advance = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
    }
    if ( _scale != 1. ) {
	for ( int i = 0; i < len; i++ )
	    glyphs[i].advance = qRound(glyphs[i].advance*_scale);
    }

    *nglyphs = len;
    return NoError;
}


void QFontEngineXft::recalcAdvances(int len, QGlyphLayout *glyphs)
{
    for ( int i = 0; i < len; i++ ) {
	FT_UInt glyph = glyphs[i].glyph;
	glyphs[i].advance = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !glyphs[i].advance ) {
	    XGlyphInfo gi;
	    XftGlyphExtents( QX11Info::appDisplay(), _font, &glyph, 1, &gi );
	    glyphs[i].advance = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		glyphs[i].advance = qRound(glyphs[i].advance*_scale);
	}
    }
}


//#define FONTENGINE_DEBUG
void QFontEngineXft::draw( QPaintEngine *p, int x, int y, const QTextItem &si, int textFlags )
{
    if ( !si.num_glyphs )
	return;

    Display *dpy = QX11Info::appDisplay();

    int xorig = x;
    int yorig = y;

    XftFont *fnt = _font;
    bool transform = FALSE;
    if ( p->painterState()->txop >= QPainter::TxScale ) {
	Q_ASSERT(_face->face_flags & FT_FACE_FLAG_SCALABLE);

	XftPattern *pattern = XftPatternDuplicate( _pattern );
	XftMatrix *mat = 0;
	XftPatternGetMatrix( pattern, XFT_MATRIX, 0, &mat );
	XftMatrix m2;
	m2.xx = p->painterState()->worldMatrix.m11()*_scale;
	m2.xy = -p->painterState()->worldMatrix.m21()*_scale;
	m2.yx = -p->painterState()->worldMatrix.m12()*_scale;
	m2.yy = p->painterState()->worldMatrix.m22()*_scale;

	// check if we have it cached
	TransformedFont *trf = transformed_fonts;
	TransformedFont *prev = 0;
	int i = 0;
	while ( trf ) {
	    if ( trf->xx == (float)m2.xx &&
		 trf->xy == (float)m2.xy &&
		 trf->yx == (float)m2.yx &&
		 trf->yy == (float)m2.yy )
		break;
	    TransformedFont *tmp = trf;
	    trf = trf->next;
	    if (i > 10) {
		XftFontClose( QX11Info::appDisplay(), tmp->xft_font );
		delete tmp;
		prev->next = trf;
	    } else {
		prev = tmp;
	    }
	    ++i;
	}
	if ( trf ) {
	    if ( prev ) {
		// move to beginning of list
		prev->next = trf->next;
		trf->next = transformed_fonts;
		transformed_fonts = trf;
	    }
	    fnt = trf->xft_font;
	} else {
	    if ( mat )
		XftMatrixMultiply( &m2, &m2, mat );

	    XftPatternDel( pattern, XFT_MATRIX );
	    XftPatternAddMatrix( pattern, XFT_MATRIX, &m2 );

	    fnt = XftFontOpenPattern( dpy, pattern );
	    TransformedFont *trf = new TransformedFont;
	    trf->xx = (float)m2.xx;
	    trf->xy = (float)m2.xy;
	    trf->yx = (float)m2.yx;
	    trf->yy = (float)m2.yy;
	    trf->xft_font = fnt;
	    trf->next = transformed_fonts;
	    transformed_fonts = trf;
	}
	transform = TRUE;
    } else if ( p->painterState()->txop == QPainter::TxTranslate ) {
	p->painterState()->painter->map( x, y, &x, &y );
    }

    QGlyphLayout *glyphs = si.glyphs;

    const QColor &pen = static_cast<QX11PaintEngine *>(p)->d->cpen.color();

    XftDraw *draw = static_cast<Q_HackPaintDevice *>(p->painterState()->painter->device())->xftDrawHandle();
    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si.num_glyphs, si.analysis.bidiLevel % 2?"true":"false" );
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, si.num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int yp = y;
    int xp = x;
#endif

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, si.width, textFlags );

    QStackArray<XftGlyphSpec,256> glyphSpec(si.num_glyphs);
    if ( si.right_to_left ) {
	int i = si.num_glyphs;
	while( i-- ) {
	    int xp = x + glyphs[i].offset.x;
	    int yp = y + glyphs[i].offset.y;
	    if ( transform )
		p->painterState()->painter->map( xp, yp, &xp, &yp );
	    glyphSpec[i].x = xp;
	    glyphSpec[i].y = yp;
	    glyphSpec[i].glyph = glyphs[i].glyph;
	    x += glyphs[i].advance;
	}
    } else {
	int i = 0;
	while ( i < si.num_glyphs ) {
	    int xp = x + glyphs[i].offset.x;
	    int yp = y + glyphs[i].offset.y;
	    if ( transform )
		p->painterState()->painter->map( xp, yp, &xp, &yp );
	    glyphSpec[i].x = xp;
	    glyphSpec[i].y = yp;
	    glyphSpec[i].glyph = glyphs[i].glyph;
	    x += glyphs[i].advance;
	    i++;
	}
    }

    XftDrawGlyphSpec( draw, &col, fnt, glyphSpec, si.num_glyphs );

#ifdef FONTENGINE_DEBUG
    if ( !si.analysis.bidiLevel % 2 ) {
	x = xp;
	y = yp;
	p->save();
	p->setPen( Qt::red );
	for ( int i = 0; i < si.num_glyphs; i++ ) {
	    glyph_metrics_t ci = boundingBox( glyphs[i] );
	    p->drawRect( x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height );
	    qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, ci.x, ci.y, ci.width, ci.height,
		   ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y, glyphs[i].advance);
	    x += glyphs[i].advance;
	}
	p->restore();
    }
#endif
}

glyph_metrics_t QFontEngineXft::boundingBox( const QGlyphLayout *glyphs, int numGlyphs )
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
	getGlyphInfo( &xgi, _font, glyphs[i].glyph );
	int x = overall.xoff + glyphs[i].offset.x - xgi.x;
	int y = overall.yoff + glyphs[i].offset.y - xgi.y;
	overall.x = qMin( overall.x, x );
	overall.y = qMin( overall.y, y );
	xmax = qMax( xmax, x + xgi.width );
	ymax = qMax( ymax, y + xgi.height );
	overall.xoff += glyphs[i].advance;
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.height = qRound(overall.height * _scale);
	overall.width = qRound(overall.width * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
    }
    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    glyph_metrics_t gm = glyph_metrics_t( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}



int QFontEngineXft::ascent() const
{
    return qRound(_font->ascent*_scale);
}

int QFontEngineXft::descent() const
{
    return qRound((_font->descent-1)*_scale);
}

// #### use Freetype to determine this
int QFontEngineXft::leading() const
{
    int l = qRound(qMin(_font->height - (_font->ascent + _font->descent),
			int(((_font->ascent + _font->descent) >> 4)*_scale)));
    return (l > 0) ? l : 1;
}

// #### use Freetype to determine this
int QFontEngineXft::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

// #### use Freetype to determine this
int QFontEngineXft::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

int QFontEngineXft::maxCharWidth() const
{
    return qRound(_font->max_advance_width*_scale);
}

static const ushort char_table[] = {
	40,
	67,
	70,
	75,
	86,
	88,
	89,
	91,
	102,
	114,
	124,
	127,
	205,
	645,
	884,
	922,
	1070,
	12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


int QFontEngineXft::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN )
	minRightBearing(); // calculates both

    return lbearing;
}

int QFontEngineXft::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	QFontEngineXft *that = (QFontEngineXft *)this;
	that->lbearing = that->rbearing = 0;
	QChar *ch = (QChar *)char_table;
	QGlyphLayout glyphs[char_table_entries];
	int ng = char_table_entries;
	stringToCMap(ch, char_table_entries, glyphs, &ng, false);
	while (--ng) {
	    if (glyphs[ng].glyph) {
		glyph_metrics_t gi = that->boundingBox( glyphs[ng].glyph );
		that->lbearing = QMIN(lbearing, gi.x);
		that->rbearing = QMIN(rbearing, gi.xoff - gi.x - gi.width);
	    }
	}
    }

    return rbearing;
}

int QFontEngineXft::cmap() const
{
    return _cmap;
}

const char *QFontEngineXft::name() const
{
    return "xft";
}

void QFontEngineXft::setScale( double scale )
{
    _scale = scale;
}

bool QFontEngineXft::canRender( const QChar *string, int len )
{
    bool allExist = TRUE;

    for ( int i = 0; i < len; i++ ) {
	if ( ! XftCharExists( QX11Info::appDisplay(), _font,
			      string[i].unicode() ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    return allExist;
}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if ( _openType )
	return _openType;

    if ( ! FT_IS_SFNT( _face ) )
	return 0;

    QFontEngineXft *that = (QFontEngineXft *)this;
    that->_openType = new QOpenType( that->_face );
    return _openType;
}


QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}
#endif


//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

#ifndef QT_NO_XFTFREETYPE

// #define OT_DEBUG

#ifdef OT_DEBUG
static inline char *tag_to_string( FT_ULong tag )
{
    static char string[5];
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
    return string;
}
#endif

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

static const unsigned int supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    FT_MAKE_TAG( 'l', 'a', 't', 'n' ),
// 	Greek,
    FT_MAKE_TAG( 'g', 'r', 'e', 'k' ),
// 	Cyrillic,
    FT_MAKE_TAG( 'c', 'y', 'r', 'l' ),
// 	Armenian,
        FT_MAKE_TAG( 'a', 'r', 'm', 'n' ),
// 	Georgian,
    FT_MAKE_TAG( 'g', 'e', 'o', 'r' ),
// 	Runic,
    FT_MAKE_TAG( 'r', 'u', 'n', 'r' ),
// 	Ogham,
    FT_MAKE_TAG( 'o', 'g', 'a', 'm' ),
// 	SpacingModifiers,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	CombiningMarks,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),

// 	// Middle Eastern Scripts
// 	Hebrew,
    FT_MAKE_TAG( 'h', 'e', 'b', 'r' ),
// 	Arabic,
    FT_MAKE_TAG( 'a', 'r', 'a', 'b' ),
// 	Syriac,
    FT_MAKE_TAG( 's', 'y', 'r', 'c' ),
// 	Thaana,
    FT_MAKE_TAG( 't', 'h', 'a', 'a' ),

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    FT_MAKE_TAG( 'd', 'e', 'v', 'a' ),
// 	Bengali,
    FT_MAKE_TAG( 'b', 'e', 'n', 'g' ),
// 	Gurmukhi,
    FT_MAKE_TAG( 'g', 'u', 'r', 'u' ),
// 	Gujarati,
    FT_MAKE_TAG( 'g', 'u', 'j', 'r' ),
// 	Oriya,
    FT_MAKE_TAG( 'o', 'r', 'y', 'a' ),
// 	Tamil,
    FT_MAKE_TAG( 't', 'a', 'm', 'l' ),
// 	Telugu,
    FT_MAKE_TAG( 't', 'e', 'l', 'u' ),
// 	Kannada,
    FT_MAKE_TAG( 'k', 'n', 'd', 'a' ),
// 	Malayalam,
    FT_MAKE_TAG( 'm', 'l', 'y', 'm' ),
// 	Sinhala,
    // ### could not find any OT specs on this
    FT_MAKE_TAG( 's', 'i', 'n', 'h' ),
// 	Thai,
    FT_MAKE_TAG( 't', 'h', 'a', 'i' ),
// 	Lao,
    FT_MAKE_TAG( 'l', 'a', 'o', ' ' ),
// 	Tibetan,
    FT_MAKE_TAG( 't', 'i', 'b', 't' ),
// 	Myanmar,
    FT_MAKE_TAG( 'm', 'y', 'm', 'r' ),
// 	Khmer,
    FT_MAKE_TAG( 'k', 'h', 'm', 'r' ),

// 	// East Asian Scripts
// 	Han,
    FT_MAKE_TAG( 'h', 'a', 'n', 'i' ),
// 	Hiragana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Katakana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Hangul,
    FT_MAKE_TAG( 'h', 'a', 'n', 'g' ),
// 	Bopomofo,
    FT_MAKE_TAG( 'b', 'o', 'p', 'o' ),
// 	Yi,
    FT_MAKE_TAG( 'y', 'i', ' ', ' ' ),

// 	// Additional Scripts
// 	Ethiopic,
    FT_MAKE_TAG( 'e', 't', 'h', 'i' ),
// 	Cherokee,
    FT_MAKE_TAG( 'c', 'h', 'e', 'r' ),
// 	CanadianAboriginal,
    FT_MAKE_TAG( 'c', 'a', 'n', 's' ),
// 	Mongolian,
    FT_MAKE_TAG( 'm', 'o', 'n', 'g' ),

// 	// Symbols
// 	CurrencySymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	LetterlikeSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	NumberForms,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MathematicalOperators,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	TechnicalSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	GeometricSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MiscellaneousSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	EnclosedAndSquare,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	Braille,
    FT_MAKE_TAG( 'b', 'r', 'a', 'i' ),

//                Unicode, should be used
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' )
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};

QOpenType::QOpenType( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
    str = tmp = 0;
    positions = 0;
    tmpAttributes = 0;
    tmpLogClusters = 0;
}

QOpenType::~QOpenType()
{
    if ( gpos )
	TT_Done_GPOS_Table( gpos );
    if ( gsub )
	TT_Done_GSUB_Table( gsub );
    if ( gdef )
	TT_Done_GDEF_Table( gdef );
    if ( str )
	TT_GSUB_String_Done( str );
    if ( tmp )
	TT_GSUB_String_Done( tmp );
    if (positions)
	free(positions);
    if ( tmpAttributes )
	free(tmpAttributes);
    if (tmpLogClusters)
	free(tmpLogClusters);
}

bool QOpenType::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script] )
	return TRUE;

#ifdef OT_DEBUG
    qDebug("trying to load tables for script %d (%s))", script, tag_to_string( supported_scripts[script] ));
#endif

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
//  	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
//  		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
//  		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
//  		qDebug("error loading gpos table: %d", error );
	    hasGPos = FALSE;
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}

bool QOpenType::loadTables( FT_ULong script)
{
    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    unsigned int stag = supported_scripts[script];

    FT_Error error = TT_GSUB_Select_Script( gsub, stag, &script_index );
    if ( error ) {
#ifdef OT_DEBUG
 	qDebug("could not select script %d: %d", (int)script, error );
#endif
	if ( stag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }
    script = stag;

#ifdef OT_DEBUG
    qDebug("script %s has script index %d", tag_to_string(script), script_index );
#endif

#ifdef OT_DEBUG
    {
	TTO_FeatureList featurelist = gsub->FeatureList;
	int numfeatures = featurelist.FeatureCount;
	qDebug("gsub table has %d features", numfeatures );
	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
	}
    }
#endif
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

#ifdef OT_DEBUG
 	qDebug("gpos table has %d features", numfeatures );
#endif

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );

#ifdef OT_DEBUG
 	    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
#endif
	}


    }

    current_script = script;

    return TRUE;
}

void QOpenType::init(QGlyphLayout *glyphs, int num_glyphs,
		     unsigned short *logClusters, int len, int /*char_offset*/)
{
    if ( !str )
	TT_GSUB_String_New(&str);
    if ( str->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( str, num_glyphs );
    if ( !tmp )
	TT_GSUB_String_New(&tmp);
    if ( tmp->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( tmp, num_glyphs );
    tmp->length = 0;

    length = len;

    Q_ASSERT(len == num_glyphs);
    tmpAttributes = (QGlyphLayout::Attributes *) realloc( tmpAttributes, num_glyphs*sizeof(QGlyphLayout::Attributes) );
    for (int i = 0; i < num_glyphs; ++i) {
	str->string[i] = glyphs[i].glyph;
	tmpAttributes[i] = glyphs[i].attributes;
    }

    for (int i = 0; i < num_glyphs; ++i)
	str->character_index[i] = i;

#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
    qDebug("log clusters before shaping:");
    for (int j = 0; j < length; j++)
	qDebug("    log[%d] = %d", j, logClusters[j] );
    qDebug("original glyphs:");
    for (int i = 0; i < num_glyphs; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmb: %d", str->string[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass);
#endif
    str->length = num_glyphs;
    orig_nglyphs = num_glyphs;

    tmpLogClusters = (unsigned short *) realloc( tmpLogClusters, length*sizeof(unsigned short) );
    memcpy( tmpLogClusters, logClusters, length*sizeof(unsigned short) );
}

void QOpenType::applyGSUBFeature(unsigned int featureTag, bool *where)
{
    FT_UShort feature_index;
    FT_Error err = TT_GSUB_Select_Feature( gsub, featureTag, script_index, 0xffff, &feature_index);
    if (err) {
#ifdef OT_DEBUG
// 	qDebug("feature %s not covered by table or language system", tag_to_string( featureTag ));
#endif
	return;
    }

#ifdef OT_DEBUG
    qDebug("applying GSUB feature %s with index %d", tag_to_string( featureTag ), feature_index);
#endif

    unsigned char w[256];
    unsigned char *where_to_apply = w;
    if (str->length > 255)
	where_to_apply = (unsigned char *)malloc(str->length*sizeof(unsigned char));

    memset(where_to_apply, 1, str->length);
    if (where) {
	int j = str->length-1;
	for (int i = orig_nglyphs-1; i >= 0; --i) {
	    if (str->character_index[j] > i)
		--j;
	    if (!where[i])
		where_to_apply[j] = 0;
	}
#ifdef OT_DEBUG
	for (int i = 0; i < (int)str->length; ++i)
	    qDebug("   apply=%s", where_to_apply[i] ? "true" : "false");
#endif
    }

    TT_GSUB_Apply_Feature(gsub, feature_index, where_to_apply, &str, &tmp);

    if (w != where_to_apply)
	free(where_to_apply);

#ifdef OT_DEBUG
    qDebug("after applying:");
    for ( int i = 0; i < (int)str->length; i++) {
      qDebug("   %4x", str->string[i]);
    }
#endif
    positioned = FALSE;
}


extern void q_heuristicPosition( QTextEngine *engine, QScriptItem *item );

void QOpenType::applyGPOSFeatures()
{
#ifdef OT_DEBUG
    qDebug("applying GPOS features");
#endif
    // currently just apply all features

    if ( hasGPos ) {
	positions = (TTO_GPOS_Data *) realloc( positions, str->length*sizeof(TTO_GPOS_Data) );
	memset(positions, 0, str->length*sizeof(TTO_GPOS_Data));

	TTO_FeatureList featurelist = gpos->FeatureList;
	int numfeatures = featurelist.FeatureCount;

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    FT_Error error = TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    if (error != FT_Err_Ok)
		continue;

#ifdef OT_DEBUG
	    qDebug("applying POS feature %s with index %d", tag_to_string( r->FeatureTag ), feature_index);
#endif

	    str->pos = 0;
	    // ### is FT_LOAD_DEFAULT the right thing to do?
	    TT_GPOS_Apply_Feature( face, gpos, feature_index, FT_LOAD_DEFAULT, str, &positions, FALSE, false );
	}
    }
    positioned = TRUE;
}

const int *QOpenType::mapping(int &len)
{
    len = str->length;
    return str->character_index;
}

void QOpenType::appendTo(QTextEngine *engine, QScriptItem *si, bool doLogClusters)
{
#ifdef OT_DEBUG
    qDebug("QOpenType::finalize:");
#endif
    // make sure we have enough space to write everything back
    engine->ensureSpace( si->num_glyphs + str->length );

    QGlyphLayout *glyphs = engine->glyphs( si ) + si->num_glyphs;

    for (unsigned int i = 0; i < str->length; ++i)
	glyphs[i].glyph = str->string[i];

    if (doLogClusters) {
	// we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
	unsigned short *logClusters = engine->logClusters( si );
	int clusterStart = 0;
	int oldCi = 0;
	for ( int i = 0; i < (int)str->length; i++ ) {
	    int ci = str->character_index[i];
	    glyphs[i].attributes = tmpAttributes[ci];
	    // 	qDebug("   ci[%d] = %d mark=%d, cmb=%d, cs=%d tmplc=%d",
	    // 	       i, ci, glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,  tmpLogClusters[ci]);
	    if ( !glyphs[i].attributes.mark && glyphs[i].attributes.clusterStart && ci != oldCi ) {
		for ( int j = oldCi; j < ci; j++ )
		    logClusters[j] = clusterStart;
		clusterStart = i;
		oldCi = ci;
	    }
	}
	for ( int j = oldCi; j < length; j++ )
	    logClusters[j] = clusterStart;
    }

    // calulate the advances for the shaped glyphs
//     qDebug("unpositioned: ");
    QFontEngineXft *font = static_cast<QFontEngineXft *>(engine->fontEngine(*si));
    font->recalcAdvances( str->length, glyphs );
    for ( int i = 0; i < (int)str->length; i++ ) {
	if ( glyphs[i].attributes.mark )
	    glyphs[i].advance = 0;
// 	    qDebug("   adv=%d", glyphs[i].advance);
    }
    si->num_glyphs += str->length;

    // positioning code:
    if ( hasGPos && positioned) {
	float scale = font->scale();
// 	qDebug("positioned glyphs:" );
	for ( int i = 0; i < (int)str->length; i++) {
// 	    qDebug("    %d:\t orig advance: %d\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 		   glyphs[i].advance, (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 		   (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 		   positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		glyphs[i].advance = qRound((positions[i].x_advance >> 6)*scale);
		//glyphs[i].advance.y = -positions[i].y_advance >> 6;
	    } else {
		glyphs[i].advance += qRound((positions[i].x_advance >> 6)*scale);
		//glyphs[i].advance.y -= positions[i].y_advance >> 6;
	    }
	    glyphs[i].offset.x = qRound((positions[i].x_pos >> 6)*scale);
	    glyphs[i].offset.y = -qRound((positions[i].y_pos >> 6)*scale);
	    int back = positions[i].back;
	    if ( si->analysis.bidiLevel % 2 ) {
		while ( back-- )
		    glyphs[i].offset.x -= glyphs[i-back].advance;
	    } else {
		while ( back )
		    glyphs[i].offset.x -= glyphs[i-(back--)].advance;
	    }
// 	    qDebug("   ->\tadv=%d\tpos=(%d/%d)",
// 		   glyphs[i].advance, glyphs[i].offset.x, glyphs[i].offset.y );
	}
	si->hasPositioning = TRUE;
    } else {
	q_heuristicPosition( engine, si );
    }

#ifdef OT_DEBUG
    qDebug("log clusters after shaping:");
    if (doLogClusters) {
	for (int j = 0; j < length; j++)
	    qDebug("    log[%d] = %d", j, engine->logClusters(si)[j] );
    }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)str->length; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d width=%d",
	       glyphs[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,
	       glyphs[i].advance);
    qDebug("-----------------------------------------");
#endif
}

#endif
