#include "fontenginexlfd.h"

#include <qstring.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

// #define FONTENGINE_DEBUG

#define ENC_BIG5_0 0
#define ENC_BIG5HKSCS_0 0
#define ENC_HKSCS_1 0
#define ENC_GB18030_0 0
#define ENC_GB18030_2000_0 0
#define ENC_GBK_0 0
#define ENC_GB2312_1980_0 0
#define ENC_ISO10646_1 0
#define ENC_ISO8859_1 0
#define ENC_ISO8859_2 0
#define ENC_ISO8859_3 0
#define ENC_ISO8859_4 0
#define ENC_ISO8859_5 0
#define ENC_ISO8859_6 0
#define ENC_ISO8859_7 0
#define ENC_ISO8859_8 0
#define ENC_ISO8859_9 0
#define ENC_ISO8859_10 0
#define ENC_ISO8859_11 0
#define ENC_ISO8859_13 0
#define ENC_ISO8859_14 0
#define ENC_ISO8859_15 0
#define ENC_JISX0201_1976_0 0
#define ENC_JISX0208_1997_0 0
#define ENC_JISX0208_1990_0 0
#define ENC_JISX0208_1983_0 0
#define ENC_KOI8_R 0
#define ENC_KOI8_RU 0
#define ENC_KOI8_U 0
#define ENC_MICROSOFT_CP1251 0
#define ENC_KSC5601_1987_0 0
#define ENC_MULELAO_1 0
#define ENC_TIS620_2533_0 0
#define ENC_TIS620_0 0
#define ENC_TSCII_0 0
#define ENC_TSCII_1 0


struct encodings {
    const char *name;
    FontMapper mapper;
};
/* maximum key range = 175, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_encoding (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181,   0,   0, 181,   0,  20,
       30,   5,  45,  25,  10,  15,   0,  50, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181,   0,   5,   0,
      181,   0,   0,   0,   5,   0,   0,   0,   0,   0,
      181,   0,   0, 181,   5,   0,   0,   0, 181, 181,
        0, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181, 181, 181, 181, 181,
      181, 181, 181, 181, 181, 181
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 16:
        hval += asso_values[(unsigned char)str[15]];
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

static inline const struct encodings *
findEncoding (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 36,
      MIN_WORD_LENGTH = 5,
      MAX_WORD_LENGTH = 16,
      MIN_HASH_VALUE = 6,
      MAX_HASH_VALUE = 180
    };

  static const struct encodings wordlist_encoding[] =
    {
      {"koi8-u", ENC_KOI8_U},
      {"tscii-0", ENC_TSCII_0},
      {"gbk-0", ENC_GBK_0},
      {"koi8-r", ENC_KOI8_R},
      {"koi8-ru", ENC_KOI8_RU},
      {"tscii-1", ENC_TSCII_1},
      {"mulelao-1", ENC_MULELAO_1},
      {"hkscs-1", ENC_HKSCS_1},
      {"big5-0", ENC_BIG5_0},
      {"gb18030-0", ENC_GB18030_0},
      {"big5hkscs-0", ENC_BIG5HKSCS_0},
      {"tis620-0", ENC_TIS620_0},
      {"gb18030.2000-0", ENC_GB18030_2000_0},
      {"iso8859-8", ENC_ISO8859_8},
      {"iso8859-3", ENC_ISO8859_3},
      {"iso8859-6", ENC_ISO8859_6},
      {"iso8859-7", ENC_ISO8859_7},
      {"iso8859-1", ENC_ISO8859_1},
      {"iso8859-10", ENC_ISO8859_10},
      {"iso8859-5", ENC_ISO8859_5},
      {"iso8859-13", ENC_ISO8859_13},
      {"iso8859-2", ENC_ISO8859_2},
      {"iso10646-1", ENC_ISO10646_1},
      {"microsoft-cp1251", ENC_MICROSOFT_CP1251},
      {"tis620.2533-0", ENC_TIS620_2533_0},
      {"jisx0208.1983-0", ENC_JISX0208_1983_0},
      {"iso8859-11", ENC_ISO8859_11},
      {"iso8859-4", ENC_ISO8859_4},
      {"iso8859-15", ENC_ISO8859_15},
      {"iso8859-9", ENC_ISO8859_9},
      {"iso8859-14", ENC_ISO8859_14},
      {"ksc5601.1987-0", ENC_KSC5601_1987_0},
      {"jisx0201.1976-0", ENC_JISX0201_1976_0},
      {"jisx0208.1990-0", ENC_JISX0208_1990_0},
      {"gb2312.1980-0", ENC_GB2312_1980_0},
      {"jisx0208.1997-0", ENC_JISX0208_1997_0}
    };

  static const signed char lookup[] =
    {
      -1, -1, -1, -1, -1, -1,  0,  1, -1, -1,  2,  3,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  5,
      -1,  6, -1, -1,  7, -1, -1, -1,  8, -1, -1,  9, -1, -1,
      -1, -1, -1, -1, 10, -1, 11, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      13, -1, -1, -1, -1, 14, -1, -1, -1, -1, 15, -1, -1, -1,
      -1, 16, -1, -1, -1, -1, 17, 18, -1, -1, -1, 19, 20, -1,
      -1, -1, 21, 22, 23, -1, 24, -1, 25, -1, -1, -1, -1, 26,
      -1, -1, -1, 27, 28, -1, -1, -1, 29, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 30, -1, -1, -1,
      31, -1, -1, -1, -1, -1, 32, -1, -1, -1, -1, 33, -1, -1,
      -1, -1, -1, -1, -1, 34, -1, -1, -1, -1, -1, -1, 35
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash_encoding (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist_encoding[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist_encoding[index];
            }
        }
    }
  return 0;
}

static inline FontMapper getMapper( const char *encoding )
{
    struct encodings *e = getMapper( encoding, strlen( encoding ) );
    if ( e )
	return e->mapper;
    return 0;
}

// returns TRUE if the character doesn't exist (ie. zero bounding box)
static inline bool charNonExistent(const XCharStruct *xcs)
{
    return (!xcs || (xcs->width == 0 && xcs->ascent + xcs->descent == 0));
}


// return the XCharStruct for the specified cell in the single dimension font xfs
static inline XCharStruct *getCharStruct1d(XFontStruct *xfs, uint c)
{
    XCharStruct *xcs = 0;
    if (c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != 0) {
	    xcs = xfs->per_char + (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = 0;
	} else
	    xcs = &(xfs->min_bounds);
    }
    return xcs;
}


// return the XCharStruct for the specified row/cell in the 2 dimension font xfs
static inline XCharStruct *getCharStruct2d(XFontStruct *xfs, uint r, uint c)
{
    XCharStruct *xcs = 0;

    if (r >= xfs->min_byte1 &&
	r <= xfs->max_byte1 &&
	c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != 0) {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = 0;
	} else
	    xcs = &(xfs->min_bounds);
    }

    return xcs;
}

static inline XCharStruct *charStruct( XFontStruct *xfs, int ch )
{
    XCharStruct *xcs;
    if (! xfs->max_byte1)
	// single row font
	xcs = getCharStruct1d(xfs, ch);
    else
	xcs = getCharStruct2d(xfs, (ch>>8), ch&0xff);
    return xcs;
}


FontEngineXLFD::FontEngineXLFD( XFontStruct *fs, const char *name, const char *encoding, int cmap )
    : _fs( fs ), _name( name ), _scale( 1. ), _cmap( cmap )
{
    fontMapper = getMapper( encoding );
}

FontEngineXLFD::~FontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
}

FontEngineIface::Error FontEngineXLFD::stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( fontMapper ) {
	fontMapper( str, glyphs, len );
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    }
    *nglyphs = len;
    return NoError;
}

void FontEngineXLFD::draw( QPainter *p, int x, int y, const GlyphIndex *glyphs,
			   const Offset *advances, const Offset *offsets, int numGlyphs, bool reverse )
{
    if ( !numGlyphs )
	return;
//     qDebug("FontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;
    Qt::BGMode bgmode = p->backgroundMode();

    Qt::HANDLE fid_last = 0;

    if (_fs->fid != fid_last) {
	XSetFont(dpy, gc, _fs->fid);
	fid_last = _fs->fid;
    }
#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    QGlyphMetrics ci = boundingBox( glyphs, advances, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    XChar2b ch[256];
    XChar2b *chars = ch;
    if ( numGlyphs > 255 )
	chars = (XChar2b *)malloc( numGlyphs*sizeof(XChar2b) );

    for (int i = 0; i < numGlyphs; i++) {
	chars[i].byte1 = glyphs[i] >> 8;
	chars[i].byte2 = glyphs[i] & 0xff;
    }

    if ( reverse ) {
	int i = numGlyphs;
	while( i-- ) {
	    Offset adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
	    QGlyphMetrics gi = boundingBox( glyphs[i] );
	    if (bgmode != Qt::TransparentMode)
		XDrawImageString16(dpy, hd, gc, x-offsets[i].x-gi.xoff, y+offsets[i].y-gi.yoff, chars+i, 1 );
	    else
		XDrawString16(dpy, hd, gc, x-offsets[i].x-gi.xoff, y+offsets[i].y-gi.yoff, chars+i, 1 );
	}
    } else {
	int i = 0;
	while( i < numGlyphs ) {
	    // ### might not work correctly with marks!
	    if (bgmode != Qt::TransparentMode)
		XDrawImageString16(dpy, hd, gc, x+offsets[i].x, y+offsets[i].y, chars+i, 1 );
	    else
		XDrawString16(dpy, hd, gc, x+offsets[i].x, y+offsets[i].y, chars+i, 1 );
	    Offset adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
	    i++;
	}
    }

    if ( numGlyphs > 255 )
	free( chars );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	QGlyphMetrics ci = boundingBox( glyphs[i] );
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

QGlyphMetrics FontEngineXLFD::boundingBox( const GlyphIndex *glyphs, const Offset *advances, const Offset *offsets, int numGlyphs )
{
    int i;

    QGlyphMetrics overall;
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
	    overall.xoff += advances[i].x;
	    overall.yoff += advances[i].y;
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

    overall.x = (int)(overall.x * _scale);
    overall.y = (int)(overall.y * _scale);
    overall.height = (int)(overall.height * _scale);
    overall.width = (int)(overall.width * _scale);
    overall.xoff = (int)(overall.xoff * _scale);
    overall.yoff = (int)(overall.yoff * _scale);
    return overall;
}

QGlyphMetrics FontEngineXLFD::boundingBox( GlyphIndex glyph )
{
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	return QGlyphMetrics( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    }
    int size = ascent();
    return QGlyphMetrics( 0, size, size, size, size, 0 );
}


int FontEngineXLFD::ascent() const
{
    return (int)(_fs->ascent*_scale);
}

int FontEngineXLFD::descent() const
{
    return (int)(_fs->descent*_scale);
}

int FontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int FontEngineXLFD::maxCharWidth() const
{
    return (int)(_fs->max_bounds.width*_scale);
}

int FontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *FontEngineXLFD::name() const
{
    return _name;
}

bool FontEngineXLFD::canRender( const QChar *string,  int len )
{
    GlyphIndex glyphs[256];
    int nglyphs = 255;
    GlyphIndex *g = glyphs;
    if ( stringToCMap( string, len, g, &nglyphs ) == OutOfMemory ) {
	g = (GlyphIndex *)malloc( nglyphs*sizeof(GlyphIndex) );
	stringToCMap( string, len, g, &nglyphs );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );
	return allExist;
}


void FontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


FontEngineIface::Type FontEngineXLFD::type() const
{
    return Xlfd;
}
