#include "qfontengine_p.h"

// #define FONTENGINE_DEBUG

#include <qcstring.h>
#include <qtextcodec.h>

#include "qbitmap.h"
#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"

#include <qt_x11_p.h>

#include "qfont.h"
#include "qtextengine_p.h"

#include <private/qunicodetables_p.h>

#include <limits.h>

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding( const char *encoding );
extern int qt_xlfd_encoding_id( const char *encoding );

extern void qt_draw_transformed_rect( QPainter *p, int x, int y, int w, int h, bool fill );

static void drawLines( QPainter *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags )
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

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    memset( glyphs, 0, len * sizeof( glyph_t ) );
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ )
	    *(advances++) = _size;
    }
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const QTextEngine *, const QScriptItem *si, int textFlags )
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif


    if ( p->txop > QPainter::TxTranslate ) {
	int xp = x;
	int yp = _size + 2;
	int s = _size - 3;
	for (int k = 0; k < si->num_glyphs; k++) {
	    qt_draw_transformed_rect( p, xp, yp, s, s, FALSE );
	    xp += _size;
	}
    } else {
	if ( p->txop == QPainter::TxTranslate )
	    p->map( x, y, &x, &y );
	XRectangle _rects[32];
	XRectangle *rects = _rects;
	if ( si->num_glyphs > 32 )
	    rects = new XRectangle[si->num_glyphs];
	for (int k = 0; k < si->num_glyphs; k++) {
	    rects[k].x = x + (k * _size);
	    rects[k].y = y - _size + 2;
	    rects[k].width = rects[k].height = _size - 3;
	}

	XDrawRectangles(dpy, hd, gc, rects, si->num_glyphs);
	if ( rects != _rects )
	    delete [] rects;
    }

    if ( textFlags != 0 )
	drawLines( p, this, y, x, si->num_glyphs*_size, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	x += offsets[i].x;
	y += offsets[i].y;
	p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offset=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y );
	x += ci.xoff;
	y += ci.yoff;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineBox::boundingBox( const glyph_t *, const advance_t *, const qoffset_t *, int numGlyphs )
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
    xlfd_transformations = XlfdTrUnknown;
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XUnloadFont( QPaintDevice::x11AppDisplay(), trf->xlfd_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	bool haveNbsp = false;
	for ( int i = 0; i < len; i++ )
	    if ( str[i].unicode() == 0xa0 ) {
		haveNbsp = TRUE;
		break;
	    }

	QChar *chars = (QChar *)str;
	if ( haveNbsp || mirrored ) {
	    chars = (QChar *)malloc( len*sizeof(QChar) );
	    for ( int i = 0; i < len; i++ )
		chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
			    (mirrored ? ::mirroredChar(str[i]).unicode() : str[i].unicode()));
	}
	_codec->fromUnicodeInternal( chars, glyphs, len );
	if (chars != str)
	    free( chars );
    } else {
	glyph_t *g = glyphs + len;
	const QChar *c = str + len;
	if ( mirrored ) {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : ::mirroredChar(*c).unicode();
	} else {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
	}
    }
    *nglyphs = len;

    if ( advances ) {
	glyph_t *g = glyphs + len;
	advance_t *a = advances + len;
	XCharStruct *xcs;
	// inlined for better perfomance
	if ( !_fs->per_char ) {
	    xcs = &_fs->min_bounds;
	    while ( a != advances )
		*(--a) = xcs->width;
	}
	else if ( !_fs->max_byte1 ) {
	    XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
	    while ( g-- != glyphs ) {
		unsigned int gl = *g;
		xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
		      base + gl : 0;
		*(--a) = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
	    }
	}
	else {
	    while ( g != glyphs ) {
		xcs = charStruct( _fs, *(--g) );
		*(--a) = (xcs ? xcs->width : _fs->ascent);
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = (int)(advances[i]*_scale);
	}
    }
    return NoError;
}

static bool x_font_load_error = FALSE;
static int x_font_errorhandler(Display *, XErrorEvent *)
{
    x_font_load_error = TRUE;
    return 0;
}


void QFontEngineXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, si->num_glyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

    bool transform = FALSE;
    int xorig = x;
    int yorig = y;

    Qt::HANDLE font_id = _fs->fid;
    if ( p->txop > QPainter::TxTranslate ) {
	if ( xlfd_transformations != XlfdTrUnsupported ) {
	    // need a transformed font from the server
	    QCString xlfd_transformed = _name;
	    int field = 0;
	    char *data = xlfd_transformed.data();
	    int pos = 0;
	    while ( field < 7 ) {
		if ( data[pos] == '-' )
		    field++;
		pos++;
	    }
	    int endPos = pos;
	    while ( data[endPos] != '-' )
		endPos++;
	    float size = xlfd_transformed.mid( pos, endPos-pos ).toInt();
	    float mat[4];
	    mat[0] = p->m11()*size*_scale;
	    mat[1] = p->m21()*size*_scale;
	    mat[2] = p->m12()*size*_scale;
	    mat[3] = p->m22()*size*_scale;

	    // check if we have it cached
	    TransformedFont *trf = transformed_fonts;
	    TransformedFont *prev = 0;
	    while ( trf ) {
		if ( trf->xx == mat[0] &&
		     trf->xy == mat[1] &&
		     trf->yx == mat[2] &&
		     trf->yy == mat[3] )
		    break;
		prev = trf;
		trf = trf->next;
	    }
	    if ( trf ) {
		if ( prev ) {
		    // move to beginning of list
		    prev->next = trf->next;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
		font_id = trf->xlfd_font;
	    } else {
		QCString matrix="[";
		for ( int i = 0; i < 4; i++ ) {
		    float f = mat[i];
		    if ( f < 0 ) {
			matrix += '~';
			f = -f;
		    }
		    matrix += QString::number( f ).latin1();
		    matrix += ' ';
		}
		matrix += ']';
		xlfd_transformed.replace( pos, endPos-pos, matrix );

		x_font_load_error = FALSE;
		XErrorHandler old_handler = XSetErrorHandler( x_font_errorhandler );
		font_id = XLoadFont( dpy, xlfd_transformed.data() );
		XSync( dpy, FALSE );
		XSetErrorHandler( old_handler );
		if ( x_font_load_error ) {
		    //qDebug( "couldn't load transformed font" );
		    font_id = _fs->fid;
		    xlfd_transformations = XlfdTrUnsupported;
		} else {
		    TransformedFont *trf = new TransformedFont;
		    trf->xx = mat[0];
		    trf->xy = mat[1];
		    trf->yx = mat[2];
		    trf->yy = mat[3];
		    trf->xlfd_font = font_id;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
	    }
	}
	if ( xlfd_transformations == XlfdTrUnsupported ) {
	    // XServer or font don't support server side transformations, need to do it by hand
            QRect bbox( 0, 0, si->width, si->ascent + si->descent + 1 );
            int w=bbox.width(), h=bbox.height();
            int aw = w, ah = h;
            int tx=-bbox.x(), ty=-bbox.y();    // text position
            QWMatrix mat1 = p->xmat;
	    if ( aw == 0 || ah == 0 )
		return;
	    double rx = (double)w / (double)aw;
	    double ry = (double)h / (double)ah;
            QWMatrix mat2 = QPixmap::trueMatrix( QWMatrix( rx, 0, 0, ry, 0, 0 )*mat1, aw, ah );
	    QBitmap *wx_bm;
#if 0
            QString bm_key = gen_text_bitmap_key( mat2, dfont, str, pos, len );
            wx_bm = get_text_bitmap( bm_key );
            bool create_new_bm = wx_bm == 0;
            if ( create_new_bm )
#endif
	    { 	        // no such cached bitmap
                QBitmap bm( aw, ah, TRUE );     // create bitmap
                QPainter paint;
                paint.begin( &bm );             // draw text in bitmap
		draw( &paint, 0, si->ascent, engine, si, textFlags );
                paint.end();
                wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
                if ( wx_bm->isNull() ) {
                    delete wx_bm;               // nothing to draw
                    return;
                }
            }
            double fx=x, fy=y - si->ascent, nfx, nfy;
            mat1.map( fx,fy, &nfx,&nfy );
            double tfx=tx, tfy=ty, dx, dy;
            mat2.map( tfx, tfy, &dx, &dy );     // compute position of bitmap
            x = qRound(nfx-dx);
            y = qRound(nfy-dy);
            XSetFillStyle( dpy, gc, FillStippled );
            XSetStipple( dpy, gc, wx_bm->handle() );
            XSetTSOrigin( dpy, gc, x, y );
            XFillRectangle( dpy, hd, gc, x, y, wx_bm->width(), wx_bm->height() );
            XSetTSOrigin( dpy, gc, 0, 0 );
            XSetFillStyle( dpy, gc, FillSolid );
#if 0
	    if ( create_new_bm )
                ins_text_bitmap( bm_key, wx_bm );
#else
	    delete wx_bm;
#endif
            return;
        }
	transform = TRUE;
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    XSetFont(dpy, gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    XChar2b ch[256];
    XChar2b *chars = ch;
    if ( si->num_glyphs > 255 )
	chars = (XChar2b *)malloc( si->num_glyphs*sizeof(XChar2b) );

    for (int i = 0; i < si->num_glyphs; i++) {
	chars[i].byte1 = glyphs[i] >> 8;
	chars[i].byte2 = glyphs[i] & 0xff;
    }

    int xpos = x;

    if ( si->analysis.bidiLevel % 2 ) {
	int i = si->num_glyphs;
	while( i-- ) {
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
	    int xp = x-offsets[i].x-gi.xoff;
	    int yp = y+offsets[i].y-gi.yoff;
	    if ( transform )
		p->map( xp, yp, &xp, &yp );
	    XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
	}
    } else {
	if ( transform || si->hasPositioning ) {
	    int i = 0;
	    while( i < si->num_glyphs ) {
		int xp = x+offsets[i].x;
		int yp = y+offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
		XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
		advance_t adv = advances[i];
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += adv;
		i++;
	    }
	} else {
	    // we can take a shortcut
	    XDrawString16(dpy, hd, gc, x, y, chars, si->num_glyphs );
	    x += si->width;
	}
    }

    if ( chars != ch )
	free( chars );

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, x-xpos, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y,
	       advances[i].x, advances[i].y);
	x += advances[i].x;
	y += advances[i].y;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineXLFD::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    int i;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	if (xcs) {
	    int x = overall.xoff + offsets[i].x - xcs->lbearing;
	    int y = overall.yoff + offsets[i].y - xcs->ascent;
	    overall.x = QMIN( overall.x, x );
	    overall.y = QMIN( overall.y, y );
	    xmax = QMAX( xmax, overall.xoff + offsets[i].x + xcs->rbearing );
	    ymax = QMAX( ymax, y + xcs->ascent + xcs->descent );
	    overall.xoff += advances[i];
	} else {
	    int size = ascent();
	    overall.x = QMIN(overall.x, overall.xoff );
	    overall.y = QMIN(overall.y, overall.yoff - size );
	    ymax = QMAX( ymax, overall.yoff );
	    overall.xoff += size;
	    xmax = QMAX( xmax, overall.xoff );
	}
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = (int)(overall.x * _scale);
	overall.y = (int)(overall.y * _scale);
	overall.height = (int)(overall.height * _scale);
	overall.width = (int)(overall.width * _scale);
	overall.xoff = (int)(overall.xoff * _scale);
	overall.yoff = (int)(overall.yoff * _scale);
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
	gm.x = (int)(gm.x * _scale);
	gm.y = (int)(gm.y * _scale);
	gm.height = (int)(gm.height * _scale);
	gm.width = (int)(gm.width * _scale);
	gm.xoff = (int)(gm.xoff * _scale);
	gm.yoff = (int)(gm.yoff * _scale);
    }
    return gm;
}


int QFontEngineXLFD::ascent() const
{
    return (int)(_fs->ascent*_scale);
}

int QFontEngineXLFD::descent() const
{
    return (int)(_fs->descent*_scale);
}

int QFontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXLFD::maxCharWidth() const
{
    return (int)(_fs->max_bounds.width*_scale);
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
		int nmx = cs[c].lbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->lbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return (int) (lbearing*_scale);
}

int QFontEngineXLFD::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->rbearing;

	    for (int c = 1; c < nc; c++) {
		int nmx = cs[c].rbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->rbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return (int) (rbearing*_scale);
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
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, FALSE );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !g[i] || !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );

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

    memset( glyphIndices, 0, sizeof( glyphIndices ) );
    memset( glyphAdvances, 0, sizeof( glyphAdvances ) );

    unsigned short chars[0x200];
    for ( int i = 0; i < 0x200; ++i )
	chars[i] = i;
    int glyphCount = 0x200;
    _engines[0]->stringToCMap( (const QChar *)chars, 0x200,
			       glyphIndices, glyphAdvances, &glyphCount, FALSE );
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

    unsigned short chars[0x200];
    glyph_t glyphs[0x200];
    advance_t advances[0x200];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    int glyphCount = 0x200;
    engine->stringToCMap( (const QChar *) chars, 0x200, glyphs, advances, &glyphCount, FALSE );

    // merge member data with the above
    for ( i = 0; i < 0x200; ++i ) {
	if ( glyphIndices[i] != 0 ) continue;
	if ( glyphs[i] == 0 ) continue;
	glyphIndices[i] = hi | glyphs[i];
	glyphAdvances[i] = advances[i];
    }
}

QFontEngine::Error
QFontEngineLatinXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs,
				    advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    int i;
    bool missing = FALSE;
    const QChar *c = str+len;
    glyph_t *g = glyphs+len;
    if ( advances ) {
	int asc = ascent();
	advance_t *a = advances+len;
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    unsigned short ch = ::mirroredChar(*c).unicode();
		    *g = glyphIndices[ch];
		    *a = glyphAdvances[ch];
		} else {
		    // #### fix for Euro et al.
		    *g = 0;
		    *a = asc;
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    *g = glyphIndices[c->unicode()];
		    *a = glyphAdvances[c->unicode()];
		} else {
		    // #### fix for Euro et al.
		    *g = 0;
		    *a = asc;
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    } else {
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		// ### fix for Euro et al.
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[::mirroredChar(*c).unicode()] : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		// ### fix for Euro et al.
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[c->unicode()] : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    }

    if ( missing ) {
	for ( i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    // #### fix for Euro
	    if ( glyphs[i] != 0 || uc >= 0x200 )
		continue;

	    QFontEngineLatinXLFD *that = (QFontEngineLatinXLFD *) this;
	    that->findEngine( str[i] );
	    glyphs[i] = that->glyphIndices[uc];
	    if ( advances )
		advances[i] = glyphAdvances[uc];
	}
    }


    *nglyphs = len;
    return NoError;
}

void QFontEngineLatinXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine,
				 const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs ) return;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    int which = glyphs[0] >> 8;

    int start = 0;
    int end, i;
    for ( end = 0; end < si->num_glyphs; ++end ) {
	const int e = glyphs[end] >> 8;
	if ( e == which ) continue;

	// set the high byte to zero
	for ( i = start; i < end; ++i )
	    glyphs[i] = glyphs[i] & 0xff;

	// draw the text
	QScriptItem si2 = *si;
	si2.glyph_data_offset = si->glyph_data_offset + start;
	si2.num_glyphs = end - start;
	_engines[which]->draw( p, x, y, engine, &si2, textFlags );

	// reset the high byte for all glyphs and advance to the next sub-string
	const int hi = which << 8;
	for ( i = start; i < end; ++i ) {
	    glyphs[i] = hi | glyphs[i];
	    x += advances[i];
	}

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    for ( i = start; i < end; ++i )
	glyphs[i] = glyphs[i] & 0xff;

    // draw the text
    QScriptItem si2 = *si;
    si2.glyph_data_offset = si->glyph_data_offset + start;
    si2.num_glyphs = end - start;
    _engines[which]->draw( p, x, y, engine, &si2, textFlags );

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for ( i = start; i < end; ++i )
	glyphs[i] = hi | glyphs[i];
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( const glyph_t *glyphs_const,
						   const advance_t *advances,
						   const qoffset_t *offsets,
						   int numGlyphs )
{
    if ( numGlyphs <= 0 ) return glyph_metrics_t();

    glyph_metrics_t overall;

    glyph_t *glyphs = (glyph_t *) glyphs_const;
    int which = glyphs[0] >> 8;

    int start = 0;
    int end, i;
    for ( end = 0; end < numGlyphs; ++end ) {
	const int e = glyphs[end] >> 8;
	if ( e == which ) continue;

	// set the high byte to zero
	for ( i = start; i < end; ++i )
	    glyphs[i] = glyphs[i] & 0xff;

	// merge the bounding box for this run
	const glyph_metrics_t gm =
	    _engines[which]->boundingBox( glyphs + start,
					  advances + start,
					  offsets + start,
					  end - start );

	overall.x = QMIN( overall.x, gm.x );
	overall.y = QMIN( overall.y, gm.y );
	overall.width = overall.xoff + gm.width;
	overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
			 QMIN( overall.y, gm.y );
	overall.xoff += gm.xoff;
	overall.yoff += gm.yoff;

	// reset the high byte for all glyphs
	const int hi = which << 8;
	for ( i = start; i < end; ++i )
	    glyphs[i] = hi | glyphs[i];

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    for ( i = start; i < end; ++i )
	glyphs[i] = glyphs[i] & 0xff;

    // merge the bounding box for this run
    const glyph_metrics_t gm =
	_engines[which]->boundingBox( glyphs + start,
				      advances + start,
				      offsets + start,
				      end - start );

    overall.x = QMIN( overall.x, gm.x );
    overall.y = QMIN( overall.y, gm.y );
    overall.width = overall.xoff + gm.width;
    overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
		     QMIN( overall.y, gm.y );
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    const int hi = which << 8;
    for ( i = start; i < end; ++i )
	glyphs[i] = hi | glyphs[i];

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
	    all = FALSE;
	    break;
	}
    }

    if ( all )
	return TRUE;

    all = TRUE;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ) continue;
	if ( glyphIndices[string[i].unicode()] != 0 ) continue;

	findEngine( string[i] );
	if ( glyphIndices[string[i].unicode()] == 0 ) {
	    all = FALSE;
	    break;
	}
    }

    return FALSE;
}

void QFontEngineLatinXLFD::setScale( double scale )
{
    for ( int i = 0; i < _count; ++i )
	_engines[i]->setScale( scale );
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

#ifdef QT_XFT2
static inline void getGlyphInfo( XGlyphInfo *xgi, XftFont *font, int glyph )
{
    FT_UInt x = glyph;
    XftGlyphExtents( QPaintDevice::x11AppDisplay(), font, &x, 1, xgi );
}
#else
static inline XftFontStruct *getFontStruct( XftFont *font )
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

static inline void getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{

    XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
}
#endif // QT_XFT2

static inline FT_Face lockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    return XftLockFace( font );
#else
    if (font->core) return 0;
    return font->u.ft.font->face;
#endif // QT_XFT2
}

static inline void unlockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    XftUnlockFace( font );
#else
    Q_UNUSED( font );
#endif // QT_XFT2
}



QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap )
{
#ifndef QT_XFT2
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( xftfs ) {
	// dirty hack: we set the charmap in the Xftfreetype to -1, so
	// XftFreetype assumes no encoding and really draws glyph
	// indices. The FT_Face still has the Unicode encoding to we
	// can convert from Unicode to glyph index
	xftfs->charmap = -1;
    }
#endif // QT_XFT2

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

    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XftFontClose( QPaintDevice::x11AppDisplay(), trf->xft_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

#ifdef QT_XFT2
    if ( mirrored ) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = ::mirroredChar(str[i]).unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		glyph_t glyph = XftCharIndex( QPaintDevice::x11AppDisplay(), _font, uc );
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[str[i].unicode()] = glyph;
	    }
	}
    } else {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		glyph_t glyph = XftCharIndex( QPaintDevice::x11AppDisplay(), _font, uc );
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[str[i].unicode()] = glyph;
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    FT_UInt glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = (int)(advances[i]*_scale);
	}
    }
#else
    if ( !_face ) {
	if ( mirrored ) {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = ::mirroredChar(str[i]).unicode();
	} else {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = str[i].unicode();
	}
    } else {
	if ( mirrored ) {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = ::mirroredChar(str[i]).unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, str[i].unicode() );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[str[i].unicode()] = glyph;
		}
	    }
	} else {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = str[i].unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, str[i].unicode() );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[str[i].unicode()] = glyph;
		}
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XftChar16 glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = (int)(advances[i]*_scale);
	}
    }
#endif // QT_XFT2

    *nglyphs = len;
    return NoError;
}


void QFontEngineXft::recalcAdvances( int len, glyph_t *glyphs, advance_t *advances )
{

#ifdef QT_XFT2
    for ( int i = 0; i < len; i++ ) {
	FT_UInt glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = (int)(advances[i]*_scale);
	}
    }
#else
    for ( int i = 0; i < len; i++ ) {
	XftChar16 glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
    }
    if ( _scale != 1. ) {
	for ( int i = 0; i < len; i++ )
	    advances[i] = (int)(advances[i]*_scale);
    }
#endif // QT_XFT2
}


//#define FONTENGINE_DEBUG
void QFontEngineXft::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

    Display *dpy = QPaintDevice::x11AppDisplay();

    int xorig = x;
    int yorig = y;

    XftFont *fnt = _font;
    bool transform = FALSE;
    if ( p->txop >= QPainter::TxScale ) {
	XftPattern *pattern = XftPatternDuplicate( _pattern );
	XftMatrix *mat = 0;
	XftPatternGetMatrix( pattern, XFT_MATRIX, 0, &mat );
	XftMatrix m2;
	m2.xx = p->m11()*_scale;
	m2.xy = p->m12()*_scale;
	m2.yx = p->m21()*_scale;
	m2.yy = p->m22()*_scale;

	// check if we have it cached
	TransformedFont *trf = transformed_fonts;
	TransformedFont *prev = 0;
	while ( trf ) {
	    if ( trf->xx == (float)m2.xx &&
		 trf->xy == (float)m2.xy &&
		 trf->yx == (float)m2.yx &&
		 trf->yy == (float)m2.yy )
		break;
	    prev = trf;
	    trf = trf->next;
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
#ifndef QT_XFT2
	    XftFontStruct *xftfs = getFontStruct( fnt );
	    if ( xftfs ) {
		// dirty hack: we set the charmap in the Xftfreetype to -1, so
		// XftFreetype assumes no encoding and really draws glyph
		// indices. The FT_Face still has the Unicode encoding to we
		// can convert from Unicode to glyph index
		xftfs->charmap = -1;
	    }
#endif // QT_XFT2
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
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    const QColor &pen = p->cpen.color();
    XftDraw *draw = ((Q_HackPaintDevice *)p->pdev)->xftDrawHandle();

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si->num_glyphs, si->analysis.bidiLevel % 2?"true":"false" );
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int yp = y;
    int xp = x;
#endif

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, si->width, textFlags );


    if ( si->isSpace )
	return;

    if ( transform || si->hasPositioning ) {
	if ( si->analysis.bidiLevel % 2 ) {
	    int i = si->num_glyphs;
	    while( i-- ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1);
#endif // QT_XFT2
#ifdef FONTENGINE_DEBUG
		glyph_metrics_t gi = boundingBox( glyphs[i] );
		p->drawRect( x+offsets[i].x+gi.x, y+offsets[i].y+100+gi.y, gi.width, gi.height );
		p->drawLine( x+offsets[i].x, y + 150 + 5*i , x+offsets[i].x+advances[i], y + 150 + 5*i );
		p->drawLine( x+offsets[i].x, y + 152 + 5*i , x+offsets[i].x+gi.xoff, y + 152 + 5*i );
		qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, gi.x, gi.y, gi.width, gi.height,
		       gi.xoff, gi.yoff, offsets[i].x, offsets[i].y, advances[i]);
#endif
		x += advances[i];
	    }
	} else {
	    int i = 0;
	    while ( i < si->num_glyphs ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1 );
#endif // QT_XFT2
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += advances[i];
		i++;
	    }
	}
    } else {
	// Xft has real trouble drawing the glyphs on their own.
	// Drawing them as one string increases performance significantly.
#ifdef QT_XFT2
	// #### we should use a different method anyways on Xft2
	FT_UInt g[128];
	FT_UInt *gl = g;
	if ( si->num_glyphs > 128 )
	    gl = new FT_UInt[si->num_glyphs];
	if ( si->analysis.bidiLevel % 2 )
	    for ( int i = 0; i < si->num_glyphs; i++ )
		gl[i] = glyphs[si->num_glyphs-1-i];
	else
	    for ( int i = 0; i < si->num_glyphs; i++ )
		gl[i] = glyphs[i];
	XftDrawGlyphs( draw, &col, fnt, x, y, gl, si->num_glyphs );
	if ( gl != g )
	    delete [] gl;
#else
	XftChar16 g[128];
	XftChar16 *gl = (XftChar16 *)glyphs;
	if ( si->analysis.bidiLevel % 2 ) {
	    gl = g;
	    if ( si->num_glyphs > 128 )
		gl = new XftChar16[si->num_glyphs];
	    for ( int i = 0; i < si->num_glyphs; i++ )
		gl[i] = glyphs[si->num_glyphs-1-i];
	}
	XftDrawString16( draw, &col, fnt, x, y, gl, si->num_glyphs );
	if ( gl != (XftChar16 *)glyphs && gl != g )
	    delete [] gl;
#endif // QT_XFT2
    }

#ifdef FONTENGINE_DEBUG
    if ( !si->analysis.bidiLevel % 2 ) {
	x = xp;
	y = yp;
	p->save();
	p->setPen( Qt::red );
	for ( int i = 0; i < si->num_glyphs; i++ ) {
	    glyph_metrics_t ci = boundingBox( glyphs[i] );
	    p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	    qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, ci.x, ci.y, ci.width, ci.height,
		   ci.xoff, ci.yoff, offsets[i].x, offsets[i].y, advances[i]);
	    x += advances[i];
	}
	p->restore();
    }
#endif
}

glyph_metrics_t QFontEngineXft::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
	getGlyphInfo( &xgi, _font, glyphs[i] );
	int x = overall.xoff + offsets[i].x - xgi.x;
	int y = overall.yoff + offsets[i].y - xgi.y;
	overall.x = QMIN( overall.x, x );
	overall.y = QMIN( overall.y, y );
	xmax = QMAX( xmax, x + xgi.width );
	ymax = QMAX( ymax, y + xgi.height );
	overall.xoff += advances[i];
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = (int)(overall.x * _scale);
	overall.y = (int)(overall.y * _scale);
	overall.height = (int)(overall.height * _scale);
	overall.width = (int)(overall.width * _scale);
	overall.xoff = (int)(overall.xoff * _scale);
	overall.yoff = (int)(overall.yoff * _scale);
    }
    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    glyph_metrics_t gm = glyph_metrics_t( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
    if ( _scale != 1. ) {
	gm.x = (int)(gm.x * _scale);
	gm.y = (int)(gm.y * _scale);
	gm.height = (int)(gm.height * _scale);
	gm.width = (int)(gm.width * _scale);
	gm.xoff = (int)(gm.xoff * _scale);
	gm.yoff = (int)(gm.yoff * _scale);
    }
    return gm;
}



int QFontEngineXft::ascent() const
{
    return (int)(_font->ascent*_scale);
}

int QFontEngineXft::descent() const
{
    return (int)(_font->descent*_scale);
}

// #### use Freetype to determine this
int QFontEngineXft::leading() const
{
    int l = qRound( (_font->ascent + _font->descent) * 0.15 * _scale );
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
    return (int)(_font->max_advance_width*_scale);
}

int QFontEngineXft::minLeftBearing() const
{
    // ### fix for Xft2
    return 0;
}

int QFontEngineXft::minRightBearing() const
{
    // ### fix for Xft2
    return 0;
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

#ifdef QT_XFT2
    for ( int i = 0; i < len; i++ ) {
	if ( ! XftCharExists( QPaintDevice::x11AppDisplay(), _font,
			      string[i].unicode() ) ) {
	    allExist = FALSE;
	    break;
	}
    }
#else
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, false ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, false );
    }

    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !XftGlyphExists(QPaintDevice::x11AppDisplay(), _font, g[i]) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );
#endif // QT_XFT2

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

static inline void tag_to_string( char *string, FT_ULong tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

struct Features {
    FT_ULong tag;
    unsigned short bit;
};

// GPOS features are always applied. We only have to note the GSUB features that should not get
// applied in all cases here.

const Features standardFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'c', 'l', 'i', 'g' ), 0x8000 },
    { 0, 0 }
};

// always keep in sync with Shape enum in scriptenginearabic.cpp
const Features arabicFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x4000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
    { FT_MAKE_TAG( 'm', 's', 'e', 't' ), 0x8000 },
    { 0, 0 }
};

const Features syriacFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '2' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '3' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'm', 'e', 'd', '2' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    { 0, 0 }
};

const Features indicFeatures[] = {
    // Language based forms
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), InitFeature },
    { FT_MAKE_TAG( 'n', 'u', 'k', 't' ), NuktaFeature },
    { FT_MAKE_TAG( 'a', 'k', 'h', 'n' ), AkhantFeature },
    { FT_MAKE_TAG( 'r', 'p', 'h', 'f' ), RephFeature },
    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), BelowFormFeature },
    { FT_MAKE_TAG( 'h', 'a', 'l', 'f' ), HalfFormFeature },
    { FT_MAKE_TAG( 'p', 's', 'b', 'f' ), PostFormFeature },
    { FT_MAKE_TAG( 'v', 'a', 't', 'u' ), VattuFeature },
    // Conjunkts and typographical forms
    { FT_MAKE_TAG( 'p', 'r', 'e', 's' ), PreSubstFeature },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstFeature },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstFeature },
    { FT_MAKE_TAG( 'p', 's', 't', 's' ), PostSubstFeature },
    // halant forms
    { FT_MAKE_TAG( 'h', 'a', 'l', 'n' ), HalantFeature },
    { 0, 0 }
};

const Features tibetanFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstFeature },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstFeature }
};


struct SupportedScript {
    FT_ULong tag;
    const Features *features;
    unsigned short required_bits;
    unsigned short always_apply;
};


const SupportedScript supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    { FT_MAKE_TAG( 'l', 'a', 't', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Greek,
    { FT_MAKE_TAG( 'g', 'r', 'e', 'k' ), standardFeatures, 0x0000, 0x8000 },
// 	Cyrillic,
    { FT_MAKE_TAG( 'c', 'y', 'r', 'l' ), standardFeatures, 0x0000, 0x8000 },
// 	Armenian,
        { FT_MAKE_TAG( 'a', 'r', 'm', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Georgian,
    { FT_MAKE_TAG( 'g', 'e', 'o', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Runic,
    { FT_MAKE_TAG( 'r', 'u', 'n', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Ogham,
    { FT_MAKE_TAG( 'o', 'g', 'a', 'm' ), standardFeatures, 0x0000, 0x8000 },
// 	SpacingModifiers,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	CombiningMarks,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },

// 	// Middle Eastern Scripts
// 	Hebrew,
    { FT_MAKE_TAG( 'h', 'e', 'b', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Arabic,
    { FT_MAKE_TAG( 'a', 'r', 'a', 'b' ), arabicFeatures, 0x400e, 0xc000 },
// 	Syriac,
    { FT_MAKE_TAG( 's', 'y', 'r', 'c' ), syriacFeatures, 0x400e, 0xc000 },
// 	Thaana,
    { FT_MAKE_TAG( 't', 'h', 'a', 'a' ), standardFeatures, 0x0000, 0x8000 },

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    { FT_MAKE_TAG( 'd', 'e', 'v', 'a' ), indicFeatures, 0x1fbe, 0x8f00 },
// 	Bengali,
    { FT_MAKE_TAG( 'b', 'e', 'n', 'g' ), indicFeatures, 0x1fff, 0x8f00 },
// 	Gurmukhi,
    { FT_MAKE_TAG( 'g', 'u', 'r', 'u' ), indicFeatures, 0x1fd2, 0x8f00 },
// 	Gujarati,
    { FT_MAKE_TAG( 'g', 'u', 'j', 'r' ), indicFeatures, 0x1fbe, 0x8f00 },
// 	Oriya,
    { FT_MAKE_TAG( 'o', 'r', 'y', 'a' ), indicFeatures, 0x1ffe, 0x8f00 },
// 	Tamil,
    { FT_MAKE_TAG( 't', 'a', 'm', 'l' ), indicFeatures, 0x1f24, 0x8f00 },
// 	Telugu,
    { FT_MAKE_TAG( 't', 'e', 'l', 'u' ), indicFeatures, 0x1e14, 0x8f00 },
// 	Kannada,
    { FT_MAKE_TAG( 'k', 'n', 'd', 'a' ), indicFeatures, 0x1e1c, 0x8f00 },
// 	Malayalam,
    { FT_MAKE_TAG( 'm', 'l', 'y', 'm' ), indicFeatures, 0x1f7c, 0x8f00 },
// 	Sinhala,
    // ### could not find any OT specs on this
    { FT_MAKE_TAG( 's', 'i', 'n', 'h' ), standardFeatures, 0x0000, 0x8000 },
// 	Thai,
    { FT_MAKE_TAG( 't', 'h', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Lao,
    { FT_MAKE_TAG( 'l', 'a', 'o', ' ' ), standardFeatures, 0x0000, 0x8000 },
// 	Tibetan,
    { FT_MAKE_TAG( 't', 'i', 'b', 't' ), tibetanFeatures, AboveSubstFeature|BelowSubstFeature, 0x8000 },
// 	Myanmar,
    { FT_MAKE_TAG( 'm', 'y', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Khmer,
    { FT_MAKE_TAG( 'k', 'h', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },

// 	// East Asian Scripts
// 	Han,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Hiragana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Katakana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Hangul,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },
// 	Bopomofo,
    { FT_MAKE_TAG( 'b', 'o', 'p', 'o' ), standardFeatures, 0x0000, 0x8000 },
// 	Yi,
    { FT_MAKE_TAG( 'y', 'i', ' ', ' ' ), standardFeatures, 0x0000, 0x8000 },

// 	// Additional Scripts
// 	Ethiopic,
    { FT_MAKE_TAG( 'e', 't', 'h', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Cherokee,
    { FT_MAKE_TAG( 'c', 'h', 'e', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	CanadianAboriginal,
    { FT_MAKE_TAG( 'c', 'a', 'n', 's' ), standardFeatures, 0x0000, 0x8000 },
// 	Mongolian,
    { FT_MAKE_TAG( 'm', 'o', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },

// 	// Symbols
// 	CurrencySymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	LetterlikeSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	NumberForms,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MathematicalOperators,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	TechnicalSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	GeometricSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MiscellaneousSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	EnclosedAndSquare,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	Braille,
    { FT_MAKE_TAG( 'b', 'r', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },

//                Unicode, should be used
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 }
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};


bool QOpenType::loadTables( FT_ULong script)
{
    always_apply = 0;

    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    const SupportedScript *s = supported_scripts + script;

    FT_Error error = TT_GSUB_Select_Script( gsub, s->tag, &script_index );
    if ( error ) {
// 	qDebug("could not select script %d: %d", (int)script, error );
	if ( s->tag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }
    script = s->tag;

//     qDebug("arabic is script %d", script_index );

    TTO_FeatureList featurelist = gsub->FeatureList;

    int numfeatures = featurelist.FeatureCount;

//     qDebug("table has %d features", numfeatures );


    found_bits = 0;
    for( int i = 0; i < numfeatures; i++ ) {
	TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	FT_ULong feature = r->FeatureTag;
	const Features *f = s->features;
	while ( f->tag ) {
	    if ( f->tag == feature ) {
		found_bits |= f->bit;
		break;
	    }
	    f++;
	}
	FT_UShort feature_index;
	TT_GSUB_Select_Feature( gsub, f->tag, script_index, DefaultLangSys,
				&feature_index );
	TT_GSUB_Add_Feature( gsub, feature_index, f->bit );

	char featureString[5];
	tag_to_string( featureString, r->FeatureTag );
// 	qDebug("found feature '%s' in GSUB table", featureString );
// 	qDebug("setting bit %x for feature, feature_index = %d", f->bit, feature_index );
    }
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select arabic script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

// 	qDebug("table has %d features", numfeatures );

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    TT_GPOS_Add_Feature( gpos, feature_index, s->always_apply );

	    char featureString[5];
	    tag_to_string( featureString, r->FeatureTag );
// 	    qDebug("found feature '%s' in GPOS table", featureString );
	}


    }
    if ( found_bits & s->required_bits != s->required_bits ) {
// 	qDebug( "not all required features for script found! found_bits=%x", found_bits );
	TT_GSUB_Clear_Features( gsub );
	return FALSE;
    }
//     qDebug("found_bits = %x", (uint)found_bits );

    always_apply = s->always_apply;
    current_script = script;

    return TRUE;
}


QOpenType::QOpenType( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
    in = out = tmp = 0;
    tmpAttributes = 0;
    tmpAttributesLen = 0;
}

QOpenType::~QOpenType()
{
    if ( gpos )
	TT_Done_GPOS_Table( gpos );
    if ( gsub )
	TT_Done_GSUB_Table( gsub );
    if ( gdef )
	TT_Done_GDEF_Table( gdef );
    if ( in )
	TT_GSUB_String_Done( in );
    if ( out )
	TT_GSUB_String_Done( out );
    if ( tmp )
	TT_GSUB_String_Done( tmp );
    if ( tmpAttributes )
	free( tmpAttributes );
}

bool QOpenType::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script].tag )
	return TRUE;

    char featureString[5];
    tag_to_string( featureString, supported_scripts[script].tag );
//     qDebug("trying to load tables for script %d (%s))", script, featureString);

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
// 	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
// 	    return FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gpos table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gpos table" );
		hasGPos = FALSE;
	    }
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}


extern void q_heuristicPosition( QTextEngine *engine, QScriptItem *item );

void QOpenType::apply( unsigned int script, unsigned short *featuresToApply, QTextEngine *engine, QScriptItem *si, int stringLength )
{
    if ( current_script != supported_scripts[script].tag ) {
	TT_GSUB_Clear_Features( gsub );

	if ( !loadTables( script ) )
	    return;
    }

    si->hasPositioning = TRUE;

    glyph_t *glyphs = engine->glyphs( si );
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );

    // shaping code

    if ( !in )
	TT_GSUB_String_New( face->memory, &in );
    if ( in->allocated < (uint)si->num_glyphs )
	TT_GSUB_String_Allocate( in, si->num_glyphs );
    if ( !out )
	TT_GSUB_String_New( face->memory, &out );
    if ( !tmp )
	TT_GSUB_String_New( face->memory, &tmp );
    out->length = 0;
    tmp->length = 0;
    out->pos = tmp->pos = 0;

    for ( int i = 0; i < si->num_glyphs; i++) {
      in->string[i] = glyphs[i];
      in->logClusters[i] = i;
      in->properties[i] = ~((featuresToApply ? featuresToApply[i] : 0)|always_apply);
    }
    in->max_ligID = 0;
    in->length = si->num_glyphs;


    TT_GSUB_Apply_String (gsub, in, &out, &tmp);

    // #### do this in place!
    if ( tmpAttributesLen < si->num_glyphs ) {
	tmpAttributes = ( GlyphAttributes *) realloc( tmpAttributes, si->num_glyphs*sizeof(GlyphAttributes) );
	tmpAttributesLen = si->num_glyphs;
    }
    memcpy( tmpAttributes, glyphAttributes, si->num_glyphs*sizeof(GlyphAttributes) );

    si->num_glyphs = out->length;
    engine->ensureSpace( si->num_glyphs );
    glyphAttributes = engine->glyphAttributes( si );

//     qDebug("out: num_glyphs = %d", si->num_glyphs );

    int clusterStart = 0;
    int oldlc = 0;
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	glyphs[i] = out->string[i];
	int lc = out->logClusters[i];
	glyphAttributes[i] = tmpAttributes[lc];
	if ( !glyphAttributes[i].mark && glyphAttributes[i].clusterStart && lc != oldlc ) {
	    for ( int j = oldlc; j < lc; j++ )
		logClusters[j] = clusterStart;
	    clusterStart = i;
	    oldlc = lc;
	}
//      	qDebug("    glyph[%d]=%4x logcluster=%d mark=%d", i, out->string[i], out->logClusters[i], glyphAttributes[i].mark );
	// ### need to fix logclusters aswell!!!!
    }
    for ( int j = oldlc; j < stringLength; j++ )
	logClusters[j] = clusterStart;
//     qDebug("log clusters after shaping:");
//     for ( int j = 0; j < stringLength; j++ )
// 	qDebug("    log[%d] = %d", j, logClusters[j] );

    // calulate the advances for the shaped glyphs
    glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    ((QFontEngineXft *)si->fontEngine)->recalcAdvances( si->num_glyphs, glyphs, advances );
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	if ( glyphAttributes[i].mark )
	    advances[i] = 0;
    }

    // positioning code:
    if ( hasGPos ) {
	TTO_GPOS_Data *positions = 0;

	bool reverse = (si->analysis.bidiLevel % 2);
	// ### is FT_LOAD_DEFAULT the right thing to do?
	TT_GPOS_Apply_String( face, gpos, FT_LOAD_DEFAULT, out, &positions, FALSE, false );

	float scale = si->fontEngine->scale();
// 	qDebug("positioned glyphs:" );
	for ( int i = 0; i < si->num_glyphs; i++) {
// 	    qDebug("    %d:\t orig advance: %d\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 		   advances[i], (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 		   (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 		   positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		advances[i] = (int)((positions[i].x_advance >> 6)*scale);
		//advances[i].y = -positions[i].y_advance >> 6;
	    } else {
		advances[i] += (int)((positions[i].x_advance >> 6)*scale);
		//advances[i].y -= positions[i].y_advance >> 6;
	    }
	    offsets[i].x = (int)((positions[i].x_pos >> 6)*scale);
	    offsets[i].y = -(int)((positions[i].y_pos >> 6)*scale);
	    int back = positions[i].back;
	    if ( reverse ) {
		while ( back-- )
		    offsets[i].x -= advances[i-back];
	    } else {
		while ( back )
		    offsets[i].x -= advances[i-(back--)];
	    }
// 	    qDebug("   ->\tadv=%d\tpos=(%d/%d)",
// 		   advances[i], offsets[i].x, offsets[i].y );
	}
	free( positions );
    } else {
	q_heuristicPosition( engine, si );
    }
}

#endif
