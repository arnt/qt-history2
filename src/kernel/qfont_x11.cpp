/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_x11.cpp#159 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Created : 940515
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qwidget.h"
#include "qpainter.h"
#include "qfontdata.h"
#include "qfontdatabase.h"
#include "qstrlist.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "qt_x11.h"
#include "qmap.h"

/* UNICODE:
     XLFD names are defined to be Latin1.
*/

static const int fontFields = 14;

enum FontFieldNames {				// X LFD fields
    Foundry,
    Family,
    Weight_,
    Slant,
    Width,
    AddStyle,
    PixelSize,
    PointSize,
    ResolutionX,
    ResolutionY,
    Spacing,
    AverageWidth,
    CharsetRegistry,
    CharsetEncoding };

// Internal functions

static bool	parseXFontName( QCString &fontName, char **tokens );
static char   **getXFontNames( const char *pattern, int *count );
static bool	smoothlyScalable( const QCString &fontName );
static bool	fontExists( const QString &fontName );
static int	getWeight( const QCString &weightString, bool adjustScore=FALSE );


#undef	IS_ZERO
#define IS_ZERO(X) (X[0] == '0' && X[1] == 0)

inline bool isScalable( char **tokens )
{
    return ( IS_ZERO(tokens[PixelSize]) &&
	     IS_ZERO(tokens[PointSize]) &&
	     IS_ZERO(tokens[AverageWidth]) );
}

inline bool isSmoothlyScalable( char **tokens )
{
    return ( IS_ZERO( tokens[ResolutionX] ) &&
	     IS_ZERO( tokens[ResolutionY] ) );
}

// QFont_Private accesses QFont protected functions

class QFont_Private : public QFont
{
public:
    int	    fontMatchScore( char *fontName, QCString &buffer,
			    float *pointSizeDiff, int *weightDiff,
			    bool *scalable, bool *polymorphic,
			    int *resx, int *resy );
    QCString bestMatch( const char *pattern, int *score );
    QCString bestFamilyMember( const char *family, int *score );
    QCString findFont( bool *exact );
    bool needsSet() const { return charSet() >= Set_1 && charSet() <= Set_N; }

};

#undef  PRIV
#define PRIV ((QFont_Private*)this)


/*****************************************************************************
  QFontInternal contains X11 font data: an XLFD font name ("-*-*-..") and
  an X font struct.

  Two global dictionaries and a cache hold QFontInternal objects, which
  are shared between all QFonts.
  This mechanism makes font loading much faster than using XLoadQueryFont.
 *****************************************************************************/

class QFontInternal
{
public:
   ~QFontInternal();
    bool	    dirty() const;
    const char	   *name()  const;
    int		    xResolution() const;
    XFontStruct	   *fontStruct() const;
    XFontSet	    fontSet() const;
    const QFontDef *spec()  const;
    int		    lineWidth() const;
    const QTextCodec *mapper() const;
    void	    reset();
private:
    QFontInternal( const QString & );
    void computeLineWidth();

    QCString	    n;
    XFontStruct	   *f;
    XFontSet	    set;
    QFontDef	    s;
    int		    lw;
    int		    xres;
    QTextCodec	   *cmapper;
    friend void QFont::load() const;
    friend void QFont::initFontInfo() const;
};

inline QFontInternal::QFontInternal( const QString &name )
    : n(name.ascii()), f(0), set(0)
{
    s.dirty = TRUE;
}

inline bool QFontInternal::dirty() const
{
    return f == 0 && set == 0;
}

inline const char *QFontInternal::name() const
{
    return n;
}

inline XFontStruct *QFontInternal::fontStruct() const
{
    return f;
}

inline XFontSet QFontInternal::fontSet() const
{
    return set;
}

inline const QFontDef *QFontInternal::spec() const
{
    return &s;
}

inline int QFontInternal::lineWidth() const
{
    return lw;
}

inline int QFontInternal::xResolution() const
{
    return xres;
}

inline const QTextCodec *QFontInternal::mapper() const
{
    return cmapper;
}

inline void QFontInternal::reset()
{
    if ( f ) {
	XFreeFont( QPaintDevice::x11AppDisplay(), f );
	f = 0;
    }
    if ( set ) {
	XFreeFontSet( QPaintDevice::x11AppDisplay(), set );
	set = 0;
    }
}

inline QFontInternal::~QFontInternal()
{
    reset();
}


static const int reserveCost   = 1024*100;
static const int fontCacheSize = 1024*1024*4;


typedef QCacheIterator<QFontInternal> QFontCacheIt;
typedef QDict<QFontInternal>	      QFontDict;
typedef QDictIterator<QFontInternal>  QFontDictIt;


class QFontCache : public QCache<QFontInternal>
{
public:
    QFontCache( int maxCost, int size=17 )
	: QCache<QFontInternal>(maxCost,size) {}
    void deleteItem( Item );
};

void QFontCache::deleteItem( Item d )
{
    QFontInternal *fin = (QFontInternal *)d;
    fin->reset();
}


struct QXFontName
{
    QXFontName( const QCString &n, bool e ) : name(n), exactMatch(e) {}
    QCString name;
    bool    exactMatch;
};

typedef QDict<QXFontName> QFontNameDict;

static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
static QFontNameDict *fontNameDict   = 0;	// dict of matched font names
                                                // default character set:
QFont::CharSet QFont::defaultCharSet = QFont::Latin1; 

//
// This function returns the X font struct for a QFontData.
// It is called from QPainter::drawText().
//

XFontStruct *qt_get_xfontstruct( QFontData *d )
{
    return d->fin->fontStruct();
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*****************************************************************************
  set_local_font() - tries to set a sensible default font char set
 *****************************************************************************/

/* This will go away - we'll just use codecForLocale() */
static struct {
    const char * name;
    QFont::CharSet cs;
} encoding_names[] = {
    { "ISO8859-1", QFont::ISO_8859_1 },
    { "ISO8859-2", QFont::ISO_8859_2 },
    { "ISO8859-3", QFont::ISO_8859_3 },
    { "ISO8859-4", QFont::ISO_8859_4 },
    { "ISO8859-5", QFont::ISO_8859_5 },
    { "ISO8859-6", QFont::ISO_8859_6 },
    { "ISO8859-7", QFont::ISO_8859_7 },
    { "ISO8859-8", QFont::ISO_8859_8 },
    { "ISO8859-9", QFont::ISO_8859_9 },
    { "ISO8859-10", QFont::ISO_8859_10 },
    { "ISO8859-11", QFont::ISO_8859_11 },
    { "ISO8859-12", QFont::ISO_8859_12 },
    { "ISO8859-13", QFont::ISO_8859_13 },
    { "ISO8859-14", QFont::ISO_8859_14 },
    { "ISO8859-15", QFont::ISO_8859_15 },
    { "KOI8-R", QFont::KOI8R },
    { "eucJP", QFont::Set_Ja },
    { "SJIS", QFont::Set_Ja },
    { "JIS7", QFont::Set_Ja },
    { "eucKR", QFont::Set_Ko },
    { "TACTIS", QFont::Set_Th_TH },
    { "eucCN", QFont::Set_Zh },
    { "eucTW", QFont::Set_Zh_TW },
    { 0, /* anything */ QFont::ISO_8859_1 }
};


/*!  
  Internal function that uses locale information to find the preferred
  character set of loaded fonts.

  \internal 
  Uses QTextCodec::codecForLocale() to find the character set name.
*/
void QFont::locale_init()
{
    QTextCodec * t = QTextCodec::codecForLocale();
    const char * p = t ? t->name() : 0;
    if ( p && *p ) {
	int i=0;
	while( encoding_names[i].name &&
	       qstricmp( p, encoding_names[i].name ) )
	    i++;
	if ( encoding_names[i].name ) {
	    defaultCharSet = encoding_names[i].cs;
	    return;
	}
    }
    defaultCharSet = QFont::Latin1;
}

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontInternal and QXFontName.
*/

void QFont::initialize()
{
    if ( fontCache )
	return;
    fontCache = new QFontCache( fontCacheSize, 29 );
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29 );
    CHECK_PTR( fontDict );
    fontNameDict = new QFontNameDict( 29 );
    CHECK_PTR( fontNameDict );
    fontNameDict->setAutoDelete( TRUE );
}

/*!
  Internal function that cleans up the font system.
*/

void QFont::cleanup()
{
    delete fontCache;
    fontCache = 0;
    fontDict->setAutoDelete( TRUE );
    delete fontDict;
    delete fontNameDict;
}

/*!
  Internal function that dumps font cache statistics.
*/

void QFont::cacheStatistics()
{
#if defined(DEBUG)
    fontCache->statistics();
    QFontCacheIt it(*fontCache);
    QFontInternal *fin;
    qDebug( "{" );
    while ( (fin = it.current()) ) {
	++it;
	qDebug( "   [%s]", fin->name() );
    }
    qDebug( "}" );
#endif
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->req.dirty || d->fin->dirty())


/*!
  Returns the window system handle to the font, for low-level
  access.  <em>Using this function is not portable.</em>
*/

HANDLE QFont::handle() const
{
    static Font last = 0;
    if ( DIRTY_FONT ) {
	load();
	if ( d->fin->fontSet() )
	    return 0;
    } else {
	if ( d->fin->fontSet() )
	    return 0;
	if ( d->fin->fontStruct()->fid != last )
	    fontCache->find( d->fin->name() );
    }

    last = d->fin->fontStruct()->fid;
    return last;
}

/*  
  Fills in a font definition (QFontDef) from an XLFD (X Logical Font
  Description). Returns TRUE if the the given xlfd is valid. If the xlfd
  is valid the encoding name (charset registry + "-" + charset encoding)
  is returned in /e encodingName if /e encodingName is non-zero. The
  fileds lbearing and rbearing are not given any values.
 */

bool fillFontDef( const QCString &xlfd, QFontDef *fd, QCString *encodingName )
{

    char *tokens[fontFields];
    QCString buffer = xlfd;
    if ( !parseXFontName(buffer, tokens) )
	return FALSE;

    if ( encodingName ) {
	*encodingName = tokens[CharsetRegistry];
	*encodingName += '-';
	*encodingName += tokens[CharsetEncoding];
    }

    fd->family    = QString::fromLatin1(tokens[Family]);
    fd->pointSize = atoi(tokens[PointSize]);
    fd->styleHint = QFont::AnyStyle;	// ### any until we match families

    if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 ) {
	if ( strcmp( tokens[CharsetEncoding], "1" ) == 0 )
	    fd->charSet = QFont::ISO_8859_1;
	else if ( strcmp( tokens[CharsetEncoding], "2" ) == 0 )
	    fd->charSet = QFont::ISO_8859_2;
	else if ( strcmp( tokens[CharsetEncoding], "3" ) == 0 )
	    fd->charSet = QFont::ISO_8859_3;
	else if ( strcmp( tokens[CharsetEncoding], "4" ) == 0 )
	    fd->charSet = QFont::ISO_8859_4;
	else if ( strcmp( tokens[CharsetEncoding], "5" ) == 0 )
	    fd->charSet = QFont::ISO_8859_5;
	else if ( strcmp( tokens[CharsetEncoding], "6" ) == 0 )
	    fd->charSet = QFont::ISO_8859_6;
	else if ( strcmp( tokens[CharsetEncoding], "7" ) == 0 )
	    fd->charSet = QFont::ISO_8859_7;
	else if ( strcmp( tokens[CharsetEncoding], "8" ) == 0 )
	    fd->charSet = QFont::ISO_8859_8;
	else if ( strcmp( tokens[CharsetEncoding], "9" ) == 0 )
	    fd->charSet = QFont::ISO_8859_9;
    } else if( strcmp( tokens[CharsetRegistry], "koi8" ) == 0 &&
	       (strcmp( tokens[CharsetEncoding], "r" ) == 0 ||
		strcmp( tokens[CharsetEncoding], "1" ) == 0) ) {
	fd->charSet = QFont::KOI8R;
    } else if( strcmp( tokens[CharsetRegistry], "iso10646" ) == 0 ) {
	fd->charSet = QFont::Unicode;
    } else {
	fd->charSet = QFont::AnyCharSet;
    }

    char slant	= tolower( tokens[Slant][0] );
    fd->italic = (slant == 'o' || slant == 'i');
    char fixed	= tolower( tokens[Spacing][0] );
    fd->fixedPitch = (fixed == 'm' || fixed == 'c');
    fd->weight = getWeight( tokens[Weight_] );

#if 0
    warning( "Resolution = %s", tokens[ResolutionY] );
    
    if ( strcmp(tokens[ResolutionY], "75") != 0 ) { // if not 75 dpi
	int fett = fd->pointSize;
	fd->pointSize = ( 2*fd->pointSize*atoi(tokens[ResolutionY]) + 1)
			   / (75 * 2);		// adjust actual pointsize
	warning( "Adjusted from %i to %i", fett, fd->pointSize  );
    }
#endif

    fd->underline     = FALSE;
    fd->strikeOut     = FALSE;
    fd->hintSetByUser = FALSE;
    fd->rawMode       = FALSE;
    fd->dirty         = FALSE;
    return TRUE;
}

/*!
  Returns the name of the font within the underlying system.
  <em>Using the return value of this function is usually not
  portable.</em>

  \sa setRawMode(), rawMode()
*/
QString QFont::rawName() const
{
    if ( DIRTY_FONT )
	load();
    return QString::fromLatin1(d->fin->name());
}

void QFont::setRawName( const QString &name )
{
    detach();
    bool validXLFD = fillFontDef( name.latin1(), &d->req, 0 );
    d->req.dirty = TRUE;
    if ( !validXLFD ) {
	setFamily( name );
	setRawMode( TRUE );
    }
}



/*!
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded, or FALSE if no changes have been made.
*/

bool QFont::dirty() const
{
    return DIRTY_FONT;
}


/*!
  Returns the family name that corresponds to the current style hint.
*/

QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return QString::fromLatin1("times");
	case Courier:
	    return QString::fromLatin1("courier");
	case Decorative:
	    return QString::fromLatin1("old english");
	case Helvetica:
	case System:
	default:
	    return QString::fromLatin1("helvetica");
    }
}

/*!
  Returns a last resort family name for the \link fontmatch.html font
  matching algorithm. \endlink

  \sa lastResortFont()
*/

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

static const char *tryFonts[] = {
    "6x13",
    "7x13",
    "8x13",
    "9x15",
    "fixed",
    "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-times-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-120-*-*-*-*-*-*",
    "-*-helvetica-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-courier-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-times-*-*-*-*-*-*-*-*-*-*-*-*",
    "-*-lucida-*-*-*-*-*-*-*-*-*-*-*-*",
    0 };

/*!
  Returns a last resort raw font name for the \link fontmatch.html font
  matching algorithm. \endlink

  This is used if not even the last resort family is available.

  \sa lastResortFamily()
*/

QString QFont::lastResortFont() const
{
    static QString last;
    if ( !last.isNull() )			// already found
	return last;
    int i = 0;
    const char* f;
    while ( (f = tryFonts[i]) ) {
	last = QString::fromLatin1(f);
	if ( fontExists(last) ) {
	    return last;
	}
	i++;
    }
#if defined(CHECK_NULL)
    qFatal( "QFont::lastResortFont: Cannot find any reasonable font" );
#endif
    return last;
}


static void resetFontDef( QFontDef *def )	// used by initFontInfo()
{
    def->pointSize     = 0;
    def->styleHint     = QFont::AnyStyle;
    def->weight	       = QFont::Normal;
    def->italic	       = FALSE;
    def->charSet       = QFont::Latin1;
    def->underline     = FALSE;
    def->strikeOut     = FALSE;
    def->fixedPitch    = FALSE;
    def->hintSetByUser = FALSE;
    def->lbearing      = SHRT_MIN;
    def->rbearing      = SHRT_MIN;
}

/*!
  Initializes the font information in the font's QFontInternal data.
  This function is called from load() for a new font.
*/

void QFont::initFontInfo() const
{
    QFontInternal *f = d->fin;
    if ( !f->s.dirty )				// already initialized
	return;

    f->s.lbearing = SHRT_MIN;
    f->s.rbearing = SHRT_MIN;
    f->computeLineWidth();

    QCString encoding;

    if (  d->exactMatch ) {
	if ( PRIV->needsSet() ) {
	    f->cmapper = QTextCodec::codecForLocale();
	} else {
	    encoding = encodingName( charSet() );
	    f->cmapper = QTextCodec::codecForName( encoding );
	}
	f->s = d->req;
	f->s.dirty = FALSE;
	return;
    }

    ASSERT(!PRIV->needsSet()); // They are always exact

    if ( fillFontDef( f->name(), &f->s, &encoding ) ) { // valid XLFD?
	f->cmapper = QTextCodec::codecForName( encoding );
    } else {
	f->cmapper = 0;
	resetFontDef( &f->s );
	f->s.family   = QString::fromLatin1(f->name());
	f->s.rawMode  = TRUE;
	d->exactMatch = FALSE;
	return;
    }
    f->s.underline = d->req.underline;
    f->s.strikeOut = d->req.strikeOut;
}

inline int maxIndex(XFontStruct *f)
{
    return
	((f->max_byte1 - f->min_byte1)
		*(f->max_char_or_byte2 - f->min_char_or_byte2 + 1)
	    + f->max_char_or_byte2 - f->min_char_or_byte2);
}


/*!
  Loads the font.
*/

void QFont::load() const
{
    if ( !fontCache )				// not initialized
	return;

    QString k = key();
    QXFontName *fn = fontNameDict->find( k );

    if ( !fn ) {
	QString name;
	bool match;
	if ( d->req.rawMode ) {
	    name = substitute( family() );
	    match = fontExists( name );
	    if ( !match )
		name = lastResortFont();
	} else {
	    name = PRIV->findFont( &match );
	}
	fn = new QXFontName( name.ascii(), match );
	CHECK_PTR( fn );
	fontNameDict->insert( k, fn );
    }
    d->exactMatch = fn->exactMatch;

    QCString n = fn->name;
    d->fin = fontCache->find( n.data() );
    if ( !d->fin ) {				// font not loaded
	d->fin = fontDict->find( n.data() );
	if ( !d->fin ) {			// font was never loaded
	    d->fin = new QFontInternal( n );
	    CHECK_PTR( d->fin );
	    fontDict->insert( d->fin->name(), d->fin );
	}
    }
    if ( PRIV->needsSet() )  {
	XFontSet s = d->fin->set;
	if ( !s ) {
	    char** missing=0;
	    int nmissing;
	    s = XCreateFontSet( QPaintDevice::x11AppDisplay(), n,
				&missing, &nmissing, 0 );
	    if ( missing )
		XFreeStringList(missing);
	    d->fin->set = s;
	    // [not cached]
	    initFontInfo();
	}
    } else {
	XFontStruct *f = d->fin->f;
	if ( !f ) {					// font not loaded
	    f = XLoadQueryFont( QPaintDevice::x11AppDisplay(), n );
	    if ( !f ) {
		f = XLoadQueryFont( QPaintDevice::x11AppDisplay(),
				    lastResortFont().ascii() );
		fn->exactMatch = FALSE;
#if defined(CHECK_NULL)
		if ( !f )
		    qFatal( "QFont::load: Internal error" );
#endif
	    }
	    int size = (f->max_bounds.ascent + f->max_bounds.descent) *
		       f->max_bounds.width * maxIndex(f) / 8;
	    // If we get a cache overflow, we make room for this font only
	    if ( size > fontCache->maxCost() + reserveCost )
		fontCache->setMaxCost( size + reserveCost );
#if defined(CHECK_STATE)
	    if ( !fontCache->insert(d->fin->name(), d->fin, size) )
		qFatal( "QFont::load: Cache overflow error" );
#endif
	    d->fin->f = f;
	    initFontInfo();
	}
    }
    d->req.dirty = FALSE;
}


/*****************************************************************************
  QFont_Private member functions
 *****************************************************************************/

#define exactScore	 0xffff

#define CharSetScore	 0x40
#define PitchScore	 0x20
#define ResolutionScore	 0x10
#define SizeScore	 0x08
#define WeightScore	 0x04
#define SlantScore	 0x02
#define WidthScore	 0x01

//
// Returns a score describing how well a font name matches the contents
// of a font.
//

int QFont_Private::fontMatchScore( char	 *fontName,	 QCString &buffer,
				   float *pointSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *polymorphic,
                                   int   *resx	       , int  *resy )
{
    char *tokens[fontFields];
    bool   exactMatch = TRUE;
    int	   score      = 0;
    *scalable	      = FALSE;
    *polymorphic      = FALSE;
    *weightDiff	      = 0;
    *pointSizeDiff    = 0;

    strcpy( buffer.data(), fontName );	// NOTE: buffer must be large enough
    if ( !parseXFontName( buffer, tokens ) )
	return 0;	// Name did not conform to X Logical Font Description

#undef	IS_ZERO
#define IS_ZERO(X) (X[0] == '0' && X[1] == 0)

    if ( IS_ZERO(tokens[Weight_]) ||
	 IS_ZERO(tokens[Slant])	  ||
	 IS_ZERO(tokens[Width]) )
	*polymorphic = TRUE;			// polymorphic font

    if ( IS_ZERO(tokens[PixelSize]) &&
	 IS_ZERO(tokens[PointSize]) &&
	 IS_ZERO(tokens[AverageWidth]) )
	*scalable = TRUE;			// scalable font

    if ( charSet() == AnyCharSet ) {
	// this can happen at least two which do not deserve warnings:
	// 1. if the program is being used in the yoo-nited states
	//    and without $LANG
	// 2. if the program explicitly asks for AnyCharSet
	score |= CharSetScore;
    } else if ( charSet() == KOI8R ) {
       if ( strcmp( tokens[CharsetRegistry], "koi8" ) == 0 &&
            (strcmp( tokens[CharsetEncoding], "r" ) == 0
             || strcmp( tokens[CharsetEncoding], "1" ) == 0) )
               score |= CharSetScore;
       else
               exactMatch = FALSE;
    } else if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 ) {
	// need to mask away non-8859 charsets here
	switch( charSet() ) {
	case ISO_8859_1:
	    if ( strcmp( tokens[CharsetEncoding], "1" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_2:
	    if ( strcmp( tokens[CharsetEncoding], "2" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_3:
	    if ( strcmp( tokens[CharsetEncoding], "3" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_4:
	    if ( strcmp( tokens[CharsetEncoding], "4" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_5:
	    if ( strcmp( tokens[CharsetEncoding], "5" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_6:
	    if ( strcmp( tokens[CharsetEncoding], "6" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_7:
	    if ( strcmp( tokens[CharsetEncoding], "7" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_8:
	    if ( strcmp( tokens[CharsetEncoding], "8" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_9:
	    if ( strcmp( tokens[CharsetEncoding], "9" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_10:
	    if ( strcmp( tokens[CharsetEncoding], "10" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_11:
	    if ( strcmp( tokens[CharsetEncoding], "11" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_12:
	    if ( strcmp( tokens[CharsetEncoding], "12" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_13:
	    if ( strcmp( tokens[CharsetEncoding], "13" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_14:
	    if ( strcmp( tokens[CharsetEncoding], "14" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case ISO_8859_15:
	    if ( strcmp( tokens[CharsetEncoding], "15" ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	default:
	    // ### should not get here
	    break;
	}
    } else if ( strcmp( tokens[CharsetRegistry], "iso10646" ) == 0 ) {
	// Yes please!
        score |= CharSetScore;
    } else {
	exactMatch = FALSE;
    }

    char pitch = tolower( tokens[Spacing][0] );
    if ( fixedPitch() ) {
	if ( pitch == 'm' || pitch == 'c' )
	    score |= PitchScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( pitch != 'p' )
	    exactMatch = FALSE;
    }

    int pSize;
    if ( *scalable )
	pSize = deciPointSize();	// scalable font
    else
	pSize = atoi( tokens[PointSize] );

    if ( strcmp(tokens[ResolutionX], "0") == 0 &&
	 strcmp(tokens[ResolutionY], "0") == 0 ) {
	// smoothly scalable font, we can ask for any resolution
	score |= ResolutionScore;
    } else {
	int localResx = atoi(tokens[ResolutionX]);
	int localResy = atoi(tokens[ResolutionY]);
	if ( *resx == 0 || *resy == 0 ) {
	    *resx = localResx;
	    *resy = localResy;
	}
	if ( localResx == *resx && localResy == *resy )
	    score |= ResolutionScore;
	else
	    exactMatch = FALSE;
#if 0
	if ( strcmp(tokens[ResolutionY], "75") != 0 ) { // if not 75 dpi
	    int fett = pSize;
	    pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 1)
			   / (75 * 2);		// adjust actual pointsize
	    warning( "X Adjusted from %i to %i", fett, pSize  );
	}
#endif
    }

    float diff;
    if ( deciPointSize() != 0 )
	diff = ((float)QABS(pSize - deciPointSize())/deciPointSize())*100.0F;
    else
	diff = (float)pSize;

    if ( diff < 20 ) {
	if ( pSize != deciPointSize() )
	    exactMatch = FALSE;
	score |= SizeScore;
    } else {
	exactMatch = FALSE;
    }
    if ( pointSizeDiff )
	*pointSizeDiff = diff;

    int weightVal = getWeight( tokens[Weight_], TRUE );

    if ( weightVal == weight() )
	score |= WeightScore;
    else
	exactMatch = FALSE;

    *weightDiff = QABS( weightVal - weight() );
    char slant = tolower( tokens[Slant][0] );
    if ( italic() ) {
	if ( slant == 'o' || slant == 'i' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    } else {
	if ( slant == 'r' )
	    score |= SlantScore;
	else
	    exactMatch = FALSE;
    }
    if ( stricmp( tokens[Width], "normal" ) == 0 )
	score |= WidthScore;
    else
	exactMatch = FALSE;
    return exactMatch ? exactScore : score;
}


struct MatchData {			// internal for bestMatch
    MatchData() { score=0; name=0; pointDiff=99; weightDiff=99; }
    int	    score;
    char   *name;
    float   pointDiff;
    int	    weightDiff;
};

QCString QFont_Private::bestMatch( const char *pattern, int *score )
{
    MatchData	best;
    MatchData	bestScalable;

    QCString	matchBuffer( 256 );	// X font name always <= 255 chars
    char **	xFontNames;
    int		count;
    int		sc;
    float	pointDiff;	// difference in % from requested point size
    int		weightDiff;	// difference from requested weight
    int		resx        = 0;
    int         resy	    = 0;
    bool	scalable    = FALSE;
    bool	polymorphic = FALSE;
    int		i;

    xFontNames = getXFontNames( pattern, &count );

    for( i = 0; i < count; i++ ) {
	resx = 0;
	resy = 0;
	// warning( "Trying %s", xFontNames[i] );
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &polymorphic, &resx, &resy );

	// warning( "Score = %i", sc );

	if ( sc > best.score ||
	     sc == best.score && pointDiff < best.pointDiff ||
	     sc == best.score && pointDiff == best.pointDiff &&
				 weightDiff < best.weightDiff ) {
	    if ( scalable ) {
		if ( sc > bestScalable.score ||
		     sc == bestScalable.score &&
			   weightDiff < bestScalable.weightDiff ) {
		    bestScalable.score	    = sc;
		    bestScalable.name	    = xFontNames[i];
		    bestScalable.pointDiff  = pointDiff;
		    bestScalable.weightDiff = weightDiff;
		}
	    } else {
		best.score	= sc;
		best.name	= xFontNames[i];
		best.pointDiff	= pointDiff;
		best.weightDiff = weightDiff;
	    }
	}
    }
    QCString bestName;
    char *tokens[fontFields];

    if ( best.score != exactScore && ( bestScalable.score > best.score ||
	     bestScalable.score == best.score && (
		 best.pointDiff != 0 ||
		 best.weightDiff < bestScalable.weightDiff ) ) ) {
	// warning( "Not exact" );
	if ( smoothlyScalable( bestScalable.name ) ) {
	    if ( resx == 0 || resy == 0 ) {
		resx = 75;
		resy = 75;
	    }
	    best.score = bestScalable.score;
	    strcpy( matchBuffer.data(), bestScalable.name );
	    if ( parseXFontName( matchBuffer, tokens ) ) {
		bestName.sprintf( "-%s-%s-%s-%s-%s-%s-*-%i-%i-%i-%s-*-%s-%s",
				  tokens[Foundry],
				  tokens[Family],
				  tokens[Weight_],
				  tokens[Slant],
				  tokens[Width],
				  tokens[AddStyle],
				  deciPointSize(),
				  resx, resy,
				  tokens[Spacing],
				  tokens[CharsetRegistry],
				  tokens[CharsetEncoding] );
		best.name = bestName.data();
	    }
	}
    }
    *score = best.score;
    bestName = best.name;

    //warning( "Best = %s", (const char *) best.name );

    XFreeFontNames( xFontNames );

    return bestName;
}


QCString QFont_Private::bestFamilyMember( const char *family, int *score )
{
    char pattern[256];
    sprintf( pattern, "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", family );
    return bestMatch( pattern, score );
}


QCString QFont_Private::findFont( bool *exact )
{
    QString familyName = family();
    *exact = TRUE;				// assume exact match
    if ( familyName.isEmpty() ) {
	familyName = defaultFamily();
	*exact = FALSE;
    }

    if ( needsSet() ) {
	// Font sets do not use scoring.
	*exact = TRUE;
	if ( familyName.length() > 32 )
	    return familyName.ascii();
	const char* wt =
	    weight() < 37
		? "light"
		: weight() < 57
		    ? "medium"
			: weight() < 69
			? "demibold"
			    : weight() < 81
			    ? "bold"
				: "black";
	const char* slant = italic() ? "i" : "r";
	const char* slant2 = italic() ? "o" : "r";
	int size = pointSize()*10;
	QCString s(512);
	s.sprintf(
	    "-*-%s-%s-%s-normal-*-*-%d-*-*-*-*-*-*,"
	    "-*-%s-*-%s-*-*-*-%d-*-*-*-*-*-*,"
	    "-*-%s-*-%s-*-*-*-%d-*-*-*-*-*-*,"
	    "-*-helvetica-%s-%s-*-*-*-%d-*-*-*-*-*-*,"
	    "-*-*-*-%s-*-*-*-%d-*-*-*-*-*-*,"
	    "-*-*-*-*-*-*-*-%d-*-*-*-*-*-*",
		familyName.ascii(), wt, slant, size,
		familyName.ascii(), slant, size,
		familyName.ascii(), slant2, size,
		slant, wt, size,
		slant, size,
		size );
	return s;
    } else {
	int score;
	QCString bestName = bestFamilyMember( familyName.ascii(), &score );
	if ( score != exactScore )
	    *exact = FALSE;

    if( score == 0 )
    {
       QString f = substitute( family() );
       if( familyName != f ) {
           familyName = f;                     // try substitution
           bestName = bestFamilyMember( familyName.ascii(), &score );
       }
    }
	if ( score == 0 ) {
	    QString f = defaultFamily();
	    if( familyName != f ) {
		familyName = f;			// try default family for style
		bestName = bestFamilyMember( familyName.ascii(), &score );
	    }
	    if ( score == 0 ) {
		f = lastResortFamily();
		if ( familyName != f ) {
		    familyName = f;			// try system default family
		    bestName = bestFamilyMember( familyName.ascii(), &score );
		}
	    }
	}
	if ( bestName.isNull() )			// no matching fonts found
	    bestName = lastResortFont().ascii();
	return bestName;
    }
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

const QFontDef *QFontMetrics::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}

void *QFontMetrics::fontStruct() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->fontStruct();
    } else {
	return fin->fontStruct();
    }
}

void *QFontMetrics::fontSet() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->fontSet();
    } else {
	return fin->fontSet();
    }
}

const QTextCodec *QFontMetrics::mapper() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->mapper();
    } else {
	return fin->mapper();
    }
}

#undef  FS
#define FS (painter ? (XFontStruct*)fontStruct() : fin->fontStruct())
#undef  SET
#define SET ((XFontSet)fontSet())

// How to calculate metrics from ink and logical rectangles.
#define LBEARING(i,l) (i.x+l.x)
#define RBEARING(i,l) (i.width-l.width)
#define ASCENT(i,l) (-i.y)
#define DESCENT(i,l) (i.height+i.y-1)


int QFontMetrics::printerAdjusted(int val) const
{
    if ( painter && painter->device() &&
	 painter->device()->devType() == QInternal::Printer) {
	painter->cfont.handle();
	int xres = painter->cfont.d->fin->xResolution();
	return qRound((val*0.75)/(xres*1.0));
    } else {
	return val;
    }
}

/*!
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
*/

int QFontMetrics::ascent() const
{
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(f->max_bounds.ascent);
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(ASCENT(ext->max_ink_extent,
				  ext->max_logical_extent));
}


/*!
  Returns the maximum descent of the font.

  The descent is the distance from the base line to the lowermost line
  where pixels may be drawn. (Note that this is different from X, which
  adds 1 pixel.)

  \sa ascent()
*/

int QFontMetrics::descent() const
{
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(f->max_bounds.descent - 1);
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(DESCENT(ext->max_ink_extent,ext->max_logical_extent));
}

inline bool inFont(XFontStruct *f, QChar ch )
{
    if ( f->max_byte1 ) {
	return ch.cell() >= f->min_char_or_byte2
	    && ch.cell() <= f->max_char_or_byte2
	    && ch.row() >= f->min_byte1
	    && ch.row() <= f->max_byte1;
    } else if ( ch.row() ) {
	uint ch16 = ch.unicode();
	return ch16 >= f->min_char_or_byte2
	    && ch16 <= f->max_char_or_byte2;
    } else {
	return ch.cell() >= f->min_char_or_byte2
	    && ch.cell() <= f->max_char_or_byte2;
    }
}

/*!
  Returns TRUE if \a ch is a valid character in the font.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    XFontStruct *f = FS;
    if ( f && !mapper() ) {
	return ::inFont(f,ch);
    }
    return TRUE; // ###### XFontSet range?  Use mapper()?
}

static
XCharStruct* charStr(const QTextCodec* mapper, XFontStruct *f, QChar ch)
{
    // Optimized - inFont() is merged in here.

    if ( !f->per_char )
	return &f->max_bounds;

    if ( mapper ) {
	int l = 1;
	QCString c = mapper->fromUnicode(ch,l);
	// #### What if c.length()>1 ?
	ch = c[0];
    }

    if ( f->max_byte1 ) {
	if ( !(ch.cell() >= f->min_char_or_byte2
	    && ch.cell() <= f->max_char_or_byte2
	    && ch.row() >= f->min_byte1
	    && ch.row() <= f->max_byte1) )
	    ch = QChar((ushort)f->default_char);
	return f->per_char +
	    ((ch.row() - f->min_byte1)
		    * (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)
		+ ch.cell() - f->min_char_or_byte2);
    } else if ( ch.row() ) {
	uint ch16 = ch.unicode();
	if ( !(ch16 >= f->min_char_or_byte2
	    && ch16 <= f->max_char_or_byte2) )
	    ch16 = f->default_char;
	return f->per_char + ch16;
    } else {
	if ( !( ch.cell() >= f->min_char_or_byte2
	    && ch.cell() <= f->max_char_or_byte2) )
	    ch = QChar((uchar)f->default_char);
	return f->per_char + ch.cell() - f->min_char_or_byte2;
    }
}

static
void getExt(QString str, int len, XRectangle& ink, XRectangle& logical, XFontSet set, const QTextCodec* m)
{
    // Callers to this / this needs to be optimized.
    // Trouble is, too much caching in multiple clients will make the
    //  overall performance suffer.

    QCString x = m->fromUnicode(str,len);
    XmbTextExtents( set, x, len, &ink, &logical );
}


/*!
  Returns the left bearing of character \a ch in the font.

  The left bearing is the rightward distance of the left-most pixel
  of the character from the logical origin of the character.
  This value is negative if the pixels of the character extend
  to the left of the logical origin.

  <em>See width(QChar) for a graphical description of this metric.</em>

  \sa rightBearing(QChar), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(QChar ch) const
{
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(charStr(mapper(),f,ch)->lbearing);

    XRectangle ink, log;
    getExt(ch,1,ink,log,SET,mapper());
    return printerAdjusted(LBEARING(ink,log));
}

/*!
  Returns the right bearing of character \a ch in the font.

  The right bearing is the leftward distance of the right-most pixel
  of the character from the logical origin of a subsequent character.
  This value is negative if the pixels of the character extend
  to the right of the width() of the character.

  <em>See width() for a graphical description of this metric.</em>

  \sa leftBearing(char), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(QChar ch) const
{
    XFontStruct *f = FS;
    if ( f ) {
	XCharStruct* cs = charStr(mapper(),f,ch);
	return printerAdjusted(cs->width - cs->rbearing);
    }
    XRectangle ink, log;
    getExt(ch,1,ink,log,SET,mapper());
    return printerAdjusted(RBEARING(ink,log));
}

/*!
  Returns the minimum left bearing of the font.

  This is the smallest leftBearing(char)
  of all characters in the font.

  \sa minRightBearing(), leftBearing(char)
*/
int QFontMetrics::minLeftBearing() const
{
    // Don't need def->lbearing, the FS stores it.
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(f->min_bounds.lbearing);
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(ext->max_logical_extent.x+ext->max_ink_extent.x);
}

/*!
  Returns the minimum right bearing of the font.

  This is the smallest rightBearing(char)
  of all characters in the font.

  \sa minLeftBearing(), rightBearing(char)
*/
int QFontMetrics::minRightBearing() const
{
    // Safely cast away const, as we cache rbearing there.
    QFontDef* def = (QFontDef*)spec();

    if ( def->rbearing == SHRT_MIN ) {
	XFontStruct *f = FS;
	if ( f ) {
	    if ( f->per_char ) {
		XCharStruct *c = f->per_char;
		int nc = maxIndex(f)+1;
		int mx = c->width - c->rbearing;
		for ( int i=1; i < nc; i++ ) {
		    int nmx = c[i].width - c[i].rbearing;
		    if ( nmx < mx )
			mx = nmx;
		}
		def->rbearing = mx;
	    } else {
		def->rbearing = f->max_bounds.width - f->max_bounds.rbearing;
	    }
	} else {
	    XFontSetExtents *ext = XExtentsOfFontSet(SET);
	    def->rbearing = ext->max_ink_extent.width
			-ext->max_logical_extent.width;
	}
    }

    return printerAdjusted(def->rbearing);
}


/*!
  Returns the height of the font.

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
*/

int QFontMetrics::height() const
{
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(f->max_bounds.ascent + f->max_bounds.descent);
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(ext->max_ink_extent.height);
}

/*!
  Returns the leading of the font.

  This is the natural inter-line spacing.

  \sa height(), lineSpacing()
*/

int QFontMetrics::leading() const
{
    XFontStruct *f = FS;
    if ( f ) {
	int l = f->ascent		 + f->descent -
		f->max_bounds.ascent - f->max_bounds.descent;
	if ( l > 0 ) {
	    return printerAdjusted(l);
	} else {
	    return 0;
	}
    }
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(ext->max_logical_extent.height
		-ext->max_ink_extent.height);
}


/*!
  Returns the distance from one base line to the next.

  This value is always equal to leading()+height().

  \sa height(), leading()
*/

int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}

/*! \overload int QFontMetrics::width( char c ) const

  Provided to aid porting from Qt 1.x.
*/

/*!
  Returns the logical width of a \e ch in pixels.  This is
  a distance appropriate for drawing a subsequent character
  after \e ch.

  <img src=bearings.gif align=right>
  Some of the metrics are described in the image to the right.
  The tall dark rectangle covers the logical width() of a character.
  The shorter
  pale rectangles cover the
  \link QFontMetrics::leftBearing() left\endlink and
  \link QFontMetrics::rightBearing() right\endlink bearings
  of the characters.  Notice that the bearings of "f" in this particular
  font are both negative, while the bearings of "o" are both positive.

  \sa boundingRect()
*/

int QFontMetrics::width( QChar ch ) const
{
    XFontStruct *f = FS;
    if ( f ) {
	return printerAdjusted(charStr(mapper(),f,ch)->width);
    } else {
	XRectangle ink, log;
	getExt(ch,1,ink,log,SET,mapper());
	return printerAdjusted(log.width);
    }
}

/*!
  Returns the width in pixels of the first \e len characters of \e str.

  If \e len is negative (default value), the whole string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() returns the distance to where the next string
  should be drawn.  Thus, width(stra)+width(strb) is always equal to
  width(stra+strb).  This is almost never the case with boundingRect().

  \sa boundingRect()
*/

int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    XFontStruct *f = FS;
    if ( f ) {
	const QTextCodec* m = mapper();
	if ( m ) {
	    return printerAdjusted(XTextWidth( f, m->fromUnicode(str,len), len ));
	} else {
	    return printerAdjusted(XTextWidth16( f, (XChar2b*)str.unicode(), len ));
	}
    }
    XRectangle ink, log;
    getExt(str,len,ink,log,SET,mapper());
    return printerAdjusted(log.width);
}


/*!
  Returns the bounding rectangle of the first \e len characters of \e str,
  which is the set of pixels the text would cover if drawn at (0,0).

  If \e len is negative (default value), the whole string is used.

  Note that the bounding rectangle may extend to the left of (0,0),
  e.g. for italicized fonts, and that the text output may cover \e all
  pixels in the bounding rectangle.

  Newline characters are processed as regular characters, \e not as
  linebreaks.

  Due to the different actual character heights, the height of the
  bounding rectangle of "Yes" and "yes" may be different.

  \sa width(), QPainter::boundingRect() */

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    // Values are printerAdjusted during calculations.

    if ( len < 0 )
	len = str.length();
    XFontStruct *f = FS;
    int direction;
    int ascent;
    int descent;
    XCharStruct overall;

    bool underline;
    bool strikeOut;
    if ( painter ) {
	underline = painter->cfont.underline();
	strikeOut = painter->cfont.strikeOut();
    } else {
	underline = underlineFlag();
	strikeOut = strikeOutFlag();
    }

    if ( f ) {
	const QTextCodec *m = mapper();
	if ( m ) {
	    XTextExtents( f, m->fromUnicode(str,len), len, &direction, &ascent, &descent, &overall );
	} else {
	    XTextExtents16( f, (XChar2b*)str.unicode(), len, &direction, &ascent, &descent, &overall );
	}
    } else {
	XRectangle ink, log;
	getExt(str,len,ink,log,SET,mapper());
	overall.lbearing = LBEARING(ink,log);
	overall.rbearing = ink.width+ink.x; // RBEARING(ink,log);
	overall.ascent = ASCENT(ink,log);
	overall.descent = DESCENT(ink,log);
	overall.width = log.width;
    }

    overall.lbearing = printerAdjusted(overall.lbearing);
    overall.rbearing = printerAdjusted(overall.rbearing);
    overall.ascent = printerAdjusted(overall.ascent);
    overall.descent = printerAdjusted(overall.descent);
    overall.width = printerAdjusted(overall.width);

    int startX = overall.lbearing;
    int width  = overall.rbearing - startX;
    ascent     = overall.ascent;
    descent    = overall.descent;
    if ( !underline && !strikeOut ) {
	width  = overall.rbearing - startX;
    } else {
	if ( startX > 0 )
	    startX = 0;
	if ( overall.rbearing < overall.width )
	   width =  overall.width - startX;
	else
	   width =  overall.rbearing - startX;
	if ( underline && len != 0 ) {
	    int ulTop = underlinePos();
	    int ulBot = ulTop + lineWidth(); // X descent is 1
	    if ( descent < ulBot )	// more than logical descent, so don't
		descent = ulBot;	// subtract 1 here!
	    if ( ascent < -ulTop )
		ascent = -ulTop;
	}
	if ( strikeOut && len != 0 ) {
	    int soTop = strikeOutPos();
	    int soBot = soTop - lineWidth(); // same as above
	    if ( descent < -soBot )
		descent = -soBot;
	    if ( ascent < soTop )
		ascent = soTop;
	}
    }
    return QRect( startX, -ascent, width, descent + ascent );
}


/*!
  Returns the width of the widest character in the font.
*/

int QFontMetrics::maxWidth() const
{
    XFontStruct *f = FS;
    if ( f )
	return printerAdjusted(f->max_bounds.width);
    XFontSetExtents *ext = XExtentsOfFontSet(SET);
    return printerAdjusted(ext->max_logical_extent.width);
}


/*!
  Returns the distance from the base line to where an underscore should be
  drawn.
  \sa strikeOutPos(), lineWidth()
*/

int QFontMetrics::underlinePos() const
{
    int pos = (lineWidth()*2 + 3)/6;
    if ( pos ) {
	return pos;
    } else {
	return 1;
    }
}


/*!
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
*/

int QFontMetrics::strikeOutPos() const
{
    XFontStruct *f = FS;
    if ( f ) {
	int pos = f->max_bounds.ascent/3;
	if ( pos ) {
	    return printerAdjusted(pos);
	} else {
	    return 1;
	}
    }
    return ascent()/3;
}


/*!
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
*/

int QFontMetrics::lineWidth() const
{
    if ( painter ) {
	painter->cfont.handle();
	return printerAdjusted(painter->cfont.d->fin->lineWidth());
    } else {
	return fin->lineWidth();
    }
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

const QTextCodec* QFontData::mapper() const
{
    return fin ? fin->mapper() : 0;
}

void* QFontData::fontSet() const
{
    return fin ? fin->fontSet() : 0;
}


/*****************************************************************************
  Internal X font functions
 *****************************************************************************/

//
// Splits an X font name into fields separated by '-'
//

static bool parseXFontName( QCString &fontName, char **tokens )
{
    if ( fontName.isEmpty() || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }
    int	  i;
    char *f = fontName.data() + 1;
    for ( i=0; i<fontFields && f && f[0]; i++ ) {
	tokens[i] = f;
	f = strchr( f, '-' );
	if( f )
	    *f++ = '\0';
    }
    if ( i < fontFields ) {
	for( int j=i ; j<fontFields; j++ )
	    tokens[j] = 0;
	return FALSE;
    }
    return TRUE;
}


//
// Get an array of X font names that matches a pattern
//

static char **getXFontNames( const char *pattern, int *count )
{
    static int maxFonts = 256;
    char **list;
    while( 1 ) {
	list = XListFonts( QPaintDevice::x11AppDisplay(), (char*)pattern,
			   maxFonts, count );
	// I know precisely why 32768 is 32768.
	if ( *count != maxFonts || maxFonts >= 32768 )
	    return list;
	XFreeFontNames( list );
	maxFonts *= 2;
    }
}


//
// Returns TRUE if the font can be smoothly scaled
//

static bool smoothlyScalable ( const QCString &/* fontName */  )
{
    return TRUE;
}


//
// Returns TRUE if the font exists, FALSE otherwise
//

static bool fontExists( const QString &fontName )
{
    char **fontNames;
    int	   count;
    fontNames = getXFontNames( fontName.ascii(), &count );
    XFreeFontNames( fontNames );
    return count != 0;
}


//
// Computes the line width (underline,strikeout) for the X font
// and fills in the X resolution of the font.
//

void QFontInternal::computeLineWidth()
{
    char *tokens[fontFields];
    QCString buffer(256);		// X font name always <= 255 chars
    strcpy( buffer.data(), name() );
    if ( !parseXFontName(buffer, tokens) ) {
	lw   = 1;                   // name did not conform to X LFD
	xres = 75;
	return;
    }
    int weight = getWeight( tokens[Weight_] );
    int pSize  = atoi( tokens[PointSize] ) / 10;
    if ( strcmp( tokens[ResolutionX], "75") != 0 || // adjust if not 75 dpi
	 strcmp( tokens[ResolutionY], "75") != 0 )
	pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 75 ) / ( 75 * 2 );
    QCString tmp = tokens[ResolutionX];
    bool ok;
    xres = tmp.toInt( &ok );
    if ( !ok || xres == 0 )
	xres = 75;		
    int score = pSize*weight;		// ad hoc algorithm
    lw = ( score ) / 700;
    if ( lw < 2 && score >= 1050 )	// looks better with thicker line
	lw = 2;				//   for small pointsizes
    if ( lw == 0 )
	lw = 1;
}


//
// Converts a weight string to a value
//

static int getWeight( const QCString &weightString, bool adjustScore )
{
    // Test in decreasing order of commonness
    //
    if ( weightString == "medium" )       return QFont::Normal;
    else if ( weightString == "bold" )    return QFont::Bold;
    else if ( weightString == "demibold") return QFont::DemiBold;
    else if ( weightString == "black" )   return QFont::Black;
    else if ( weightString == "light" )   return QFont::Light;

    QCString s = weightString;
    s = s.lower();
    if ( s.contains("bold") ) {
	if ( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
	else
	    return (int) QFont::Bold;
    }
    if ( s.contains("light") ) {
	if ( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
       else
	    return (int) QFont::Light;
    }
    if ( s.contains("black") ) {
	if ( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
	else
	    return (int) QFont::Black;
    }
    if ( adjustScore )
	return (int) QFont::Normal - 2;	   // - 2, we hope it's close to normal

    return (int) QFont::Normal;
}

/*--------------------------------------------------------------------------
  -------------------------- QFontDatabase ---------------------------------
  --------------------------------------------------------------------------*/

static char **xFontList = 0;
static int xFontCount = 0;

static QFont::CharSet getCharSet( const char * registry, const char *encoding);
static QFont::CharSet getCharSet( const QString &name );
static QString getStyleName( char ** tokens, bool *italic,bool *lesserItalic );
static QString getCharSetName( const char * registry, const char *encoding );
static QString getCharSetName( QFont::CharSet cs );

template<class T>
T *findName( const QList<T> &list, const QString &name )
{
    QListIterator<T> iter(list);

    T *tmp;
    for( ; (tmp = iter.current()) ; ++iter )
	if ( tmp->name() == name )
	    return tmp;
    return 0;
}

#include "qfontdatabase.h"

class QtFontCharSet;
class QtFontFamily;
class QtFontFoundry;

class QtFontStyle
{
public:
    QtFontStyle( QtFontCharSet *prnt, const QString &n )
	               { p                = prnt;
		         nm		  = n;
			 bitmapScalable   = FALSE;
			 smoothlyScalable = FALSE;
			 weightDirty      = TRUE;
			 ital             = FALSE;
			 lesserItal       = FALSE;
			 weightVal        = 0;
			 weightDirty      = TRUE;
			 sizesDirty       = TRUE; }

    QFont font( int pointSize ) const;

    const QString &name() const { return nm; }
    const QtFontCharSet *parent() const { return p; }

    const QValueList<int> &pointSizes() const;
    const QValueList<int> &smoothSizes() const;
    static const QValueList<int> &standardSizes();

    int weight() const;
    bool italic() const { return ital; }
    bool lesserItalic() const { return lesserItal; }

    bool isBitmapScalable() const { return bitmapScalable; }
    bool isSmoothlyScalable() const { return smoothlyScalable; }


private:
    const QValueList<int> &storedSizes() const;

    void addPointSize( int size );
    void setSmoothlyScalable();
    void setBitmapScalable();


    QtFontCharSet *p;
    QString nm;

    bool bitmapScalable;
    bool smoothlyScalable;

    bool ital;
    bool lesserItal;
    QCString weightString;
    int  weightVal;
    bool weightDirty;
    bool sizesDirty;

    QMap<int, int> sizeMap;
    QValueList<int> sizeList;
    
    friend void QFontDatabase::createDatabase();
};

class QtFontCharSet {
public:
    QtFontCharSet( QtFontFamily *prnt, const QString n )
                          { p                = prnt;
			    nm               = n;
			    // charSet          = QFont::AnyCharSet;
			    dirty            = TRUE;
			    namesDirty       = TRUE;
                            bitmapScalable   = FALSE;
                            smoothlyScalable = FALSE;
			    normalStyle      = 0;
			    italicStyle      = 0;
			    boldStyle        = 0;
			    italicBoldStyle  = 0;
			    chSetDirty	     = TRUE;
			    chSet	     = QFont::AnyCharSet;
			  }


    const QString &name() const { return nm; }
    QFont::CharSet charSet() const;

    const QtFontFamily *parent() const { return p; }

    const QStringList &styles() const;
    const QtFontStyle *style( const QString &name ) const;

    bool isLocaleCharSet() const;
    bool isUnicode() const;

    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;

private:
    void refresh() const;

    QtFontFamily *p;
    QString nm;

    bool dirty;
    bool namesDirty;
    bool bitmapScalable;
    bool smoothlyScalable;

    bool chSetDirty;
    QFont::CharSet chSet;

    QtFontStyle *normalStyle; // Only makes sense if the font is scalable
    QtFontStyle *italicStyle; // Gives information about which
    QtFontStyle *boldStyle;   //  combinations of these are available.
    QtFontStyle *italicBoldStyle;


    void addStyle( QtFontStyle *style )
	{ styleDict.insert( style->name(), style );  }

    QDict<QtFontStyle> styleDict;
    QStringList styleNames;

    friend void QFontDatabase::createDatabase();
};

class QtFontFamily
{
public:
    QtFontFamily( QtFontFoundry *prnt, const QString &n )
	{ p                = prnt; 
          nm               = n;
	  namesDirty       = TRUE;
          bitmapScalable   = FALSE;
          smoothlyScalable = FALSE;
          scalableDirty    = TRUE;
          localeDirty      = TRUE;
	  supportsLocale   = FALSE;
        }

    const QString &name() const { return nm; }

    const QtFontFoundry *parent() { return p; }

    const QStringList &charSets( bool onlyForLocale = TRUE ) const;
    const QtFontCharSet *charSet( const QString &n = QString::null ) const;
    
    bool isBitmapScalable() const;
    bool isSmoothlyScalable() const;

    bool hasLocaleCharSet() const;
    bool supportsCharSet( QFont::CharSet chSet ) const;

private:
    void refresh() const;
    void addCharSet( QtFontCharSet *c )
	{ charSetDict.insert( c->name(), c ); }

    QString nm;
    QtFontFoundry *p;

    // QList<QtFontCharSet> charSets;
    QStringList charSetNames;
    QDict<QtFontCharSet> charSetDict;

    bool namesDirty;
    bool localeDirty;
    bool scalableDirty;

    bool bitmapScalable;
    bool smoothlyScalable;
    bool supportsLocale;

    friend void QFontDatabase::createDatabase();
};

class QtFontFoundry
{
public:
    QtFontFoundry( const QString &n ) { nm = n; namesDirty = TRUE; }

    const QString name() const { return nm; }

    const QStringList &families() const;
    const QtFontFamily *family( const QString &name ) const;

private:
    QString nm;

    QStringList familyNames;
    QDict<QtFontFamily> familyDict;

    bool namesDirty;

    void addFamily( QtFontFamily *f )
	{ familyDict.insert( f->name(), f ); }

    friend void QFontDatabase::createDatabase();
};

class QFontDatabasePrivate {
public:
    QFontDatabasePrivate(){
	namesDirty  	    = TRUE;
	familiesDirty  	    = TRUE;
	foundryDict.setAutoDelete( TRUE );
    }

    const QStringList &families( bool onlyForLocale ) const;
    const QtFontFamily *family( const QString &name ) const;

    const QStringList &foundries() const;
    const QtFontFoundry *foundry( const QString foundryName ) const;

private:
    QStringList foundryNames;
    QDict<QtFontFoundry> foundryDict;

    QStringList familyNames;
    QDict<QtFontFamily> bestFamilyDict;

    bool namesDirty;
    bool familiesDirty;

    void addFoundry( QtFontFoundry *f )
	{ foundryDict.insert( f->name(), f ); }
    
    friend void QFontDatabase::createDatabase();
};

QFont QtFontStyle::font( int pointSize ) const
{
    QString family         = parent()->parent()->name();
    QFont::CharSet charSet = getCharSet( parent()->name() );  // ### fttb

    QFont f( family, pointSize, weight(), italic() );
    f.setCharSet( charSet );
    return f;
}

const QValueList<int> &QtFontStyle::pointSizes() const
{
#if 0
    if ( smoothlyScalable || bitmapScalable )
#else 
    if ( smoothlyScalable )
#endif
	return standardSizes();
    else
	return storedSizes();
}

const QValueList<int> &QtFontStyle::smoothSizes() const
{
    if ( smoothlyScalable )
	return standardSizes();
    else
	return storedSizes();
}

int QtFontStyle::weight() const
{
    if ( weightDirty ) {
	QtFontStyle *that = (QtFontStyle*)this; // mutable function
	that->weightVal = getWeight( weightString, TRUE );
	that->weightDirty = FALSE;
    }
    return weightVal;
}

const QValueList<int> &QtFontStyle::storedSizes() const
{
    if ( sizesDirty ) {
	QtFontStyle *that = (QtFontStyle*)this;  // Mutable function
	QMap<int, int>::ConstIterator it = sizeMap.begin();
	for( ; it != sizeMap.end() ; ++it )
	    that->sizeList.append( *it );
	that->sizesDirty = FALSE;
    }
    return sizeList;
}

const QValueList<int> &QtFontStyle::standardSizes()
{
    static int s[]={ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28,
		     36, 48, 72, 0 };
    static bool first = TRUE;
    static QValueList<int> sList;
    if ( first ) {
	first = FALSE;
	int i = 0;
	while( s[i] )
	    sList.append( s[i++] );
    }
    return sList;
}

void QtFontStyle::addPointSize( int pointSize )
{
    if ( smoothlyScalable )
	return;
    sizeMap.insert( pointSize, pointSize );
}

void QtFontStyle::setSmoothlyScalable()
{
    smoothlyScalable = TRUE;
    sizeMap.clear();
}

void QtFontStyle::setBitmapScalable()
{
    bitmapScalable = TRUE;
}

int styleSortValue( QtFontStyle *style )
{
    int score = 0;
    if ( style->weight() == QFont::Bold )
	score += 100;
    else if ( style->weight() != QFont::Normal )
	score += 1000;

    if ( style->italic() ) {
	score += 10;
	if ( style->lesserItalic() )
	    score += 1000;
    }
    return score;

}

QFont::CharSet QtFontCharSet::charSet() const
{
    if ( chSetDirty ) {
	QtFontCharSet *that = (QtFontCharSet*)this;  // Mutable function
	that->chSet      = getCharSet( name() );
	that->chSetDirty = FALSE;
    }
    return chSet;
}

const QStringList &QtFontCharSet::styles() const
{
    if ( namesDirty ) {
	QtFontCharSet *that = (QtFontCharSet*) this;  // Mutable function
	QMap<int, QString> styleMap;
	QDictIterator<QtFontStyle> iter( styleDict );
	QtFontStyle *tmp;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    styleMap.insert( styleSortValue( tmp ), tmp->name() );
	}
	QMap<int,QString>::Iterator it = styleMap.begin();
	for ( ; it != styleMap.end(); ++it )
	    that->styleNames.append( *it );
	that->namesDirty = FALSE;
    }
    return styleNames;
}

const QtFontStyle *QtFontCharSet::style( const QString &s ) const
{
    return styleDict.find( s );
}

bool QtFontCharSet::isLocaleCharSet() const
{			      
    return charSet() == QFont::charSetForLocale() || isUnicode();
}

bool QtFontCharSet::isUnicode() const
{			      
    return charSet() == QFont::Unicode;
}

bool QtFontCharSet::isBitmapScalable() const
{			      
    refresh();
    return bitmapScalable;
}

bool QtFontCharSet::isSmoothlyScalable() const
{			      
    refresh();
    return smoothlyScalable;
}

/*!
  Traverses all styles. If all of them are scalable, scalable is set to
  TRUE, if all of them are smoothly scalable smoothlyScalable is set to
  TRUE.

  The styles that most closely resemble a normal, italic, bold and bold
  italc font are found.
*/
void QtFontCharSet::refresh() const
{
    if ( !dirty )
	return;
    QtFontCharSet *that = (QtFontCharSet*)this; // mutable function
    that->smoothlyScalable = FALSE;
    that->bitmapScalable   = FALSE;

    that->normalStyle       = 0;
    that->italicStyle       = 0;
    that->boldStyle         = 0;
    that->italicBoldStyle   = 0;

    QtFontStyle *lesserItalicStyle     = 0;
    QtFontStyle *lesserItalicBoldStyle = 0;

    bool smooth = TRUE;
    bool bitmap = TRUE;
                 // Anything bolder than Normal qualifies as bold:
    int  bestBoldDiff             = QFont::Bold - QFont::Normal;
    int  bestItalicBoldDiff       = QFont::Bold - QFont::Normal;
    int  bestLesserItalicBoldDiff = QFont::Bold - QFont::Normal;
    int  bestNormal               = 0;
    int  bestItalicNormal         = 0;
    int  bestLesserItalicNormal   = 0;
    int  boldDiff;
    QtFontStyle *tmp;
    QDictIterator<QtFontStyle> iter(styleDict);
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->isSmoothlyScalable() ) {
	    smooth = FALSE;
	    if ( !tmp->isBitmapScalable() )
		bitmap = FALSE;
	}
	if ( tmp->italic() ) {
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestItalicNormal ) {
		    that->italicStyle      = tmp;
		    bestItalicNormal = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestItalicBoldDiff ) {
		    that->italicBoldStyle    = tmp;
		    bestItalicBoldDiff = boldDiff;
		}
		
	    }
	} else if ( tmp->lesserItalic() ){
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestLesserItalicNormal ) {
		    lesserItalicStyle      = tmp;
		    bestLesserItalicNormal = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestItalicBoldDiff ) {
		    lesserItalicBoldStyle    = tmp;
		    bestLesserItalicBoldDiff = boldDiff;
		}
		
	    }
	} else {
	    if ( tmp->weight() < QFont::Normal ) {
		if ( tmp->weight() > bestNormal ) {
		    that->normalStyle = tmp;
		    bestNormal  = tmp->weight();
		}
	    } else {
		boldDiff = abs( tmp->weight() - QFont::Bold );
		if ( boldDiff < bestBoldDiff ) {
		    that->boldStyle    = tmp;
		    bestBoldDiff = boldDiff;
		}
		
	    }
	}
    }
    if ( !that->italicStyle && lesserItalicStyle )
	that->italicStyle = lesserItalicStyle;
    if ( !that->italicBoldStyle && lesserItalicBoldStyle )
	that->italicBoldStyle = lesserItalicBoldStyle;
    if ( smooth )
	that->smoothlyScalable = TRUE;
    else if ( bitmap )
	that->bitmapScalable = TRUE;
    that->dirty    = FALSE;
}	

const QStringList &QtFontFamily::charSets( bool onlyForLocale ) const
{
    if ( namesDirty ) {
	QtFontFamily *that = (QtFontFamily*)this; // mutable function
	QDictIterator<QtFontCharSet> iter( charSetDict );
	QtFontCharSet *tmp;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    if ( !onlyForLocale || tmp->isLocaleCharSet() )
		that->charSetNames.append( tmp->name() );
	}
	that->charSetNames.sort();
	that->namesDirty = FALSE;
    }
    return charSetNames;
}

QString localCharSet() // ###
{
    return "iso8859-1";
}

const QtFontCharSet *QtFontFamily::charSet( const QString &n ) const
{
    if ( n.isEmpty() )
	return charSetDict.find ( localCharSet() );
    else
	return charSetDict.find ( n );
}

bool QtFontFamily::isBitmapScalable() const
{			      
    refresh();
    return bitmapScalable;
}

bool QtFontFamily::isSmoothlyScalable() const
{			      
    refresh();
    return smoothlyScalable;
}

bool QtFontFamily::hasLocaleCharSet() const
{
    if ( localeDirty ) {
	QtFontFamily *that   = (QtFontFamily*)this; // mutable function
	QDictIterator<QtFontCharSet> iter( charSetDict );
	QtFontCharSet *tmp;
	that->supportsLocale = FALSE;
	for( ; (tmp = iter.current()) ; ++iter ) {
	    if ( tmp->isLocaleCharSet() ) {
		that->supportsLocale = TRUE;
		break;
	    }
	}
	that->localeDirty = FALSE;
    }
    return supportsLocale;
}

bool QtFontFamily::supportsCharSet( QFont::CharSet chSet ) const
{
    QDictIterator<QtFontCharSet> iter( charSetDict );
    QtFontCharSet *tmp;
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( tmp->charSet() == chSet )
	    return TRUE;;
    }
    return FALSE;
}

void QtFontFamily::refresh() const
{
    if ( !scalableDirty )
	return;
    QtFontFamily *that = (QtFontFamily*) this;   // Mutable function
    that->scalableDirty    = FALSE;
    that->smoothlyScalable = FALSE;
    that->bitmapScalable   = FALSE;

    bool isSmooth = TRUE;
    QtFontCharSet *tmp;
    QDictIterator<QtFontCharSet> iter(charSetDict);
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->isSmoothlyScalable() ) {
	    isSmooth = FALSE;
	    if ( !tmp->isBitmapScalable() )
		return;
	}
    }
    if ( isSmooth )
	that->smoothlyScalable = TRUE;
    else
	that->bitmapScalable   = TRUE;
}	

const QStringList &QtFontFoundry::families() const
{
    if ( namesDirty ) {
	QtFontFoundry *that = (QtFontFoundry*) this;   // Mutable function
	QDictIterator<QtFontFamily> iter( familyDict );
	QtFontFamily *tmp;
	for( ; (tmp = iter.current()) ; ++iter )
	    that->familyNames.append( tmp->name() );
	that->familyNames.sort();
	that->namesDirty = FALSE;
    }
    return familyNames;
}

const QtFontFamily *QtFontFoundry::family( const QString &n ) const
{
    return familyDict.find( n );
}

bool localeNeedsSet()
{
    return QFont::charSetForLocale() >= QFont::Set_1 &&
           QFont::charSetForLocale() <= QFont::Set_N;
}

const QStringList &QFontDatabasePrivate::families( bool onlyForLocale ) const
{
    QFontDatabasePrivate *that = (QFontDatabasePrivate*)this; // Mutable
    if ( familiesDirty ) {
	QDict<QtFontFoundry> firstFoundryForFamily;
	QDict<int> doubles;
	QtFontFoundry *foundry;
	QString s;
	QDictIterator<QtFontFoundry> iter( foundryDict );
	for( ; (foundry = iter.current()) ; ++iter ) {
	    QStringList l = foundry->families();
	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
		if ( onlyForLocale ) {
		     const QtFontFamily *tmp = foundry->family( *it );
		     if ( !tmp ) {
			 qWarning( "QFontDatabasePrivate::families:"
				   "Internal error, %s not found.",
				   (const char*)*it );
			 continue;
		     }
		     if ( !localeNeedsSet() && !tmp->hasLocaleCharSet() )
			 continue;
		}
		if ( !firstFoundryForFamily.find( *it ) ) {
		    that->familyNames.append( *it );
		    firstFoundryForFamily.insert( *it, foundry );
		} else {
		    QString s;
		    if ( !doubles.find(*it) ) { // 2nd foundry for family?
			doubles.insert( *it, (int*)1 );
			QtFontFoundry *tmp = firstFoundryForFamily.find(*it);
			QString firstFoundryName;
			if ( tmp )
			    firstFoundryName = tmp->name();
			else
			    qWarning( "QFontDatabasePrivate::families:"
				  "Internal error, Cannot find first foundry");

			that->familyNames.remove( *it );
			s = *it + " [" + firstFoundryName + "]";
			that->familyNames.append( s );
		    }
		    s = *it + " [" + foundry->name() + "]";
		    that->familyNames.append( s );
		}
	    }
	}
	that->familyNames.sort();
	that->familiesDirty = FALSE;
    }
    return familyNames;
}


const QtFontFamily *QFontDatabasePrivate::family( const QString &name ) const
{
    if ( name.isEmpty() )
	return 0;
    QFontDatabasePrivate *that = (QFontDatabasePrivate*)this; // Mutable    
    const QtFontFamily *result = bestFamilyDict.find(name);
    if ( !result ) {
	const QtFontFoundry *fndry;
	const QtFontFamily *fam;

	if ( name.contains('[') ) {
	    int start = name.find( '[' );
	    int stop = name.find( ']' );
	    if ( start != -1 && stop != -1 && stop > start ) {
		QString foundryName = name.mid( start + 1, stop - start - 1 )
				      .stripWhiteSpace();
		QString familyName = name.left( start ).stripWhiteSpace();
		fndry = foundry( foundryName );
		if ( fndry ) {
		    fam = fndry->family( familyName );
		    if ( fam ) {
			that->bestFamilyDict.insert( name, fam );
			return fam;
		    }
		    
		}
	    }
	}
	QList<QtFontFamily> list;
	QDictIterator<QtFontFoundry> iter( foundryDict );
	const QtFontFamily *nonScalable    = 0;
	const QtFontFamily *bitmapScalable = 0;
	QString s;
	for( ; (fndry = iter.current()) ; ++iter ) {
	    fam = fndry->family( name );
	    if ( fam ) {
		if ( fam->isSmoothlyScalable() ) {
		    result = fam;
		    break;
		}
		if ( fam->isBitmapScalable() )
		    bitmapScalable = fam;
		else 
		    nonScalable    = fam;
	    }		
	}
	if ( !result )
	    result = bitmapScalable ? bitmapScalable : nonScalable;
	if ( result )
	    that->bestFamilyDict.insert( name, result );
    }
    return result;
}

const QStringList &QFontDatabasePrivate::foundries() const
{
    if ( namesDirty ) {
	QFontDatabasePrivate *that = (QFontDatabasePrivate*) this;  // Mutable
	QDictIterator<QtFontFoundry> iter( foundryDict );
	QtFontFoundry *tmp;
	for( ; (tmp = iter.current()) ; ++iter )
	    that->foundryNames.append( tmp->name() );
	that->foundryNames.sort();
	that->namesDirty = FALSE;
    }
    return foundryNames;
    
}

const QtFontFoundry *QFontDatabasePrivate::foundry( const QString foundryName ) const
{
    return foundryDict.find( foundryName );
}

static QFontDatabasePrivate *db=0;

void QFontDatabase::createDatabase()
{
    if ( db ) return;

    db = new QFontDatabasePrivate;

    xFontList = XListFonts( qt_xdisplay(), "*", 32767, &xFontCount );

    if ( xFontCount >= 32767 )
	qWarning( "More than 32k fonts, please notify qt-bugs@troll.no" );

    char *tokens[fontFields];

    for( int i = 0 ; i < xFontCount ; i++ ) {
	QCString fName = xFontList[i];
	if ( parseXFontName( fName, tokens ) ) {
	    QString foundryName = tokens[Foundry];
	    QtFontFoundry *foundry = db->foundryDict.find( foundryName );
	    if ( !foundry ) {
		//warning( "New font foundry [%s]", (const char*) foundryName );
		foundry = new QtFontFoundry( foundryName );
		CHECK_PTR(foundry);
		db->addFoundry( foundry );
	    }
	    QString familyName = tokens[Family];
	    QtFontFamily *family = foundry->familyDict.find( familyName );
	    if ( !family ) {
		//warning( "New font family [%s][%s]",
		// (const char*) familyName, (const char*) foundryName );
		family = new QtFontFamily( foundry, familyName );
		CHECK_PTR(family);
		foundry->addFamily( family );
	    }
	    QString charSetName = getCharSetName( tokens[CharsetRegistry],
						 tokens[CharsetEncoding] );
	    QtFontCharSet *charSet = family->charSetDict.find( charSetName );
	    if ( !charSet ) {
		//warning( "New charset[%s] for family [%s][%s]",
		// (const char*)charSetName, (const char *)familyName,
		// (const char *)foundryName );
		charSet = new QtFontCharSet( family, charSetName );
		CHECK_PTR(charSet);
		family->addCharSet( charSet );
	    }
	    bool italic;
	    bool lesserItalic;
	    QString styleName = getStyleName( tokens, &italic, &lesserItalic );
	    QtFontStyle *style = charSet->styleDict.find( styleName );
	    if ( !style ) {
		//warning( "New style[%s] for [%s][%s][%s]",
		// (const char*)styleName, (const char*)charSetName,
		// (const char*)familyName, (const char *)foundryName );
		style = new QtFontStyle( charSet, styleName );
		CHECK_PTR( style );
		style->ital         = italic;
		style->lesserItal   = lesserItalic;
		style->weightString = tokens[Weight_];
		charSet->addStyle( style );
	    }
	    if ( ::isScalable(tokens) ) {
		if ( ::isSmoothlyScalable( tokens ) ) {
		    style->setSmoothlyScalable();
		    //warning( "Smooth [%s][%s][%s]", (const char*) styleName,
		    //     (const char*)charSetName,
		    //     tokens[Family] );
		} else {
		    style->setBitmapScalable();
		    //warning( "Scalable, [%s][%s]", (const char*)charSetName,
		    //     tokens[Family] );
		}
	    } else {
		QCString ps = tokens[PointSize];
		int pSize = ps.toInt()/10;
		if ( pSize != 0 ) {
		    style->addPointSize( pSize );
		}
	    }
	} else {
	    //warning( "createDatabase: Not XLFD[%s]", xFontList[i] );
	}
    }
}

static QFont::CharSet getCharSet( const char * registry, const char *encoding )
{
    if ( strcmp( registry, "iso8859" ) == 0 ) {
	if ( encoding[0] != 0 && encoding[1] == 0 ) {
	    switch( encoding[0] ) {	
	    case '1': return QFont::ISO_8859_1;
	    case '2': return QFont::ISO_8859_2;
	    case '3': return QFont::ISO_8859_3;
	    case '4': return QFont::ISO_8859_4;
	    case '5': return QFont::ISO_8859_5;
	    case '6': return QFont::ISO_8859_6;
	    case '7': return QFont::ISO_8859_7;
	    case '8': return QFont::ISO_8859_8;
	    case '9': return QFont::ISO_8859_9;
	    default: break;
	    }
	} else if ( encoding[0] == '1' && encoding[1] != 0 
		    && encoding[2] == 0 ) {
	    switch( encoding[0] ) {	
	    case '0': return QFont::ISO_8859_10;
	    case '1': return QFont::ISO_8859_11;
	    case '2': return QFont::ISO_8859_12;
	    case '3': return QFont::ISO_8859_13;
	    case '4': return QFont::ISO_8859_14;
	    case '5': return QFont::ISO_8859_15;
	    default: break;
	    }
	}
	return QFont::AnyCharSet;
    } else if ( strcmp( registry, "koi8" ) == 0 &&
		(strcmp( encoding, "r" ) == 0 ||
		 strcmp( encoding, "1" ) == 0) ) {
	return QFont::KOI8R;
    } else if ( strcmp( registry, "iso10646" ) == 0 ) {
	return QFont::Unicode;
    }
    return QFont::AnyCharSet;
}

static QFont::CharSet getCharSet( const QString &name )
{
    if ( name == "iso8859-1" )
	return QFont::ISO_8859_1;
    if ( name == "iso8859-2" )
	return QFont::ISO_8859_2;
    if ( name == "iso8859-3" )
	return QFont::ISO_8859_3;
    if ( name == "iso8859-4" )
	return QFont::ISO_8859_4;
    if ( name == "iso8859-5" )
	return QFont::ISO_8859_5;
    if ( name == "iso8859-6" )
	return QFont::ISO_8859_6;
    if ( name == "iso8859-7" )
	return QFont::ISO_8859_7;
    if ( name == "iso8859-8" )
	return QFont::ISO_8859_8;
    if ( name == "iso8859-9" )
	return QFont::ISO_8859_9;
    if ( name == "iso8859-10" )
	return QFont::ISO_8859_10;
    if ( name == "iso8859-11" )
	return QFont::ISO_8859_11;
    if ( name == "iso8859-12" )
	return QFont::ISO_8859_12;
    if ( name == "iso8859-13" )
	return QFont::ISO_8859_13;
    if ( name == "iso8859-14" )
	return QFont::ISO_8859_14;
    if ( name == "iso8859-15" )
	return QFont::ISO_8859_15;
    if ( name == "koi8-r" )
	return QFont::KOI8R;
    if ( name == "koi8-1" )
	return QFont::KOI8R;
    if ( name == "iso10646" )
	return QFont::Unicode;
    return QFont::AnyCharSet;
}

static QString getCharSetName( const char * registry, const char *encoding )
{
    QString tmp = registry;
    tmp += "-";
    tmp += encoding;
    return tmp.lower();
}

static QString getCharSetName( QFont::CharSet cs )
{
    const char* name=0;
    switch( cs ) {
    case QFont::ISO_8859_1:
	name = "Western (ISO 8859-1)";
	break;
    case QFont::ISO_8859_2:
	name = "Eastern European (ISO 8859-2)";
	break;
    case QFont::ISO_8859_3:
	name = "Esperanto and more (ISO 8859-3)";
	break;
    case QFont::ISO_8859_4:
	name = "(ISO 8859-4)";
	break;
    case QFont::ISO_8859_5:
	name = "Cyrillic (ISO 8859-5)";
	break;
    case QFont::ISO_8859_6:
	name = "Arabic (ISO 8859-6)";
	break;
    case QFont::ISO_8859_7:
	name = "Greek (ISO 8859-7)";
	break;
    case QFont::ISO_8859_8:
	name = "Hebrew (ISO 8859-8)";
	break;
    case QFont::ISO_8859_9:
	name = "Turkish(ISO 8859-9)";
	break;
    case QFont::ISO_8859_10:
	name = "Nordic(ISO 8859-10)";
	break;
    case QFont::ISO_8859_11:
	name = "Thai(ISO 8859-11)";
	break;
    case QFont::ISO_8859_12:
	name = "Devanagari(Hindi)(ISO 8859-12)";
	break;
    case QFont::ISO_8859_13:
	name = "Baltic(ISO 8859-13)";
	break;
    case QFont::ISO_8859_14:
	name = "Celtic(ISO 8859-14)";
	break;
    case QFont::ISO_8859_15:
	name = "French/Finnish/Euro(ISO 8859-15)";
	break;
    case QFont::KOI8R:
	name = "Cyrillic (KOI8-R)";
	break;
    case QFont::Unicode:
	name = "Unicode (ISO 10646)";
	break;
    default:
	qWarning( "getCharSetName: Internal error, unknown charset (%i).", cs );
	name = "Unknown";
	break;
    }
    return qApp ? qApp->translate("QFont", name) : QString::fromLatin1(name);
}

static QString getStyleName( char ** tokens, bool *italic, bool *lesserItalic )
{
    char slant0	= tolower( tokens[Slant][0] );
    *italic      = FALSE;
    *lesserItalic = FALSE;

    QString nm = QString::fromLatin1(tokens[Weight_]);

    if ( nm == QString::fromLatin1("medium") )
	nm = QString::fromLatin1("");
    if ( nm.length() > 0 )
	nm.replace( 0, 1, QString(nm[0]).upper());

    if ( slant0 == 'r' ) {
	if ( tokens[Slant][1]) {
	    char slant1 = tolower( tokens[Slant][1] );
	    if ( slant1 == 'o' ) {
		nm += ' ';
		nm += qApp->translate("QFont","Reverse Oblique");
		*italic       = TRUE;
		*lesserItalic = TRUE;
	    } else if ( slant0 == 'i' ) {
		nm += ' ';
		nm += qApp->translate("QFont","Reverse Italic");
		*italic       = TRUE;
		*lesserItalic = TRUE;
	    }
	} else {
	    // Normal
	}
    } else if ( slant0 == 'o' ) {
	nm += ' ';
	if ( tokens[Slant][1] ) {
	    nm += qApp->translate("QFont","Other");
	} else {
	    nm += qApp->translate("QFont","Oblique");
	    *italic = TRUE;
	}
    } else if ( slant0 == 'i' ) {
	nm += ' ';
	nm += qApp->translate("QFont","Italic");
	*italic = TRUE;
    }
    if ( nm.isEmpty() ) {
	nm = qApp->translate("QFont","Normal");
    } else if ( nm[0] == ' ' ) {
	nm = nm.remove( 0, 1 );
    }
    return nm;
}

static QStringList emptyList;

QFontDatabase::QFontDatabase()
{
    createDatabase();
    d = db;
}

const QStringList &QFontDatabase::families( bool onlyForLocale ) const
{
    return d->families( onlyForLocale );
}


const QStringList &QFontDatabase::styles( const QString &family,
					  const QString &charSet ) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return emptyList;
    const QtFontCharSet * chSet = fam->charSet( charSet );;
    return chSet ? chSet->styles() : emptyList;
}


bool  QFontDatabase::isBitmapScalable( const QString &family,
				       const QString &style,
				       const QString &charSet ) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;
    if ( style.isEmpty() )
	return fam->isBitmapScalable();
    const QtFontCharSet * chSet = fam->charSet( charSet );;
    if ( !chSet )
	return FALSE;
    if ( style.isEmpty() )
	return chSet->isBitmapScalable();
    const QtFontStyle *sty = chSet->style( style );
    return sty && sty->isBitmapScalable();
}

bool  QFontDatabase::isSmoothlyScalable( const QString &family,
					 const QString &style,
					 const QString &charSet ) const
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return FALSE;
    if ( style.isEmpty() )
	return fam->isSmoothlyScalable();
    const QtFontCharSet * chSet = fam->charSet( charSet );;
    if ( !chSet )
	return FALSE;
    if ( style.isEmpty() )
	return chSet->isSmoothlyScalable();
    const QtFontStyle *sty = chSet->style( style );
    return sty && sty->isSmoothlyScalable();
}

bool  QFontDatabase::isScalable( const QString &family,
				 const QString &style,
				 const QString &charSet ) const
{
    if ( isSmoothlyScalable( family, style, charSet) )
	return TRUE;
    return isBitmapScalable( family, style, charSet );
}

static const QtFontStyle * getStyle( QFontDatabasePrivate *d,
				     const QString &family,
				     const QString &style,
				     const QString &charSet )
{
    const QtFontFamily *fam = d->family( family );
    if ( !fam )
	return 0;
    const QtFontCharSet * chSet = fam->charSet( charSet );
    if ( !chSet )
	return 0;
    return chSet->style( style );
}

static QValueList<int> emptySizeList;

const QValueList<int> QFontDatabase::pointSizes( const QString &family,
						 const QString &style,
						 const QString &charSet )
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty ? sty->pointSizes() : emptySizeList;
}


QFont QFontDatabase::font( const QString family, const QString &style,
			   int pointSize, const QString charSet )
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty->font( pointSize );
}

/*!  
  Returns the point sizes of a font style that is guaranteed to look
  good. For non-scalable fonts and smoothly scalable fonts this function
  is equivalent to pointSizes().
*/

const QValueList<int> QFontDatabase::smoothSizes( const QString &family,
						  const QString &style,
						  const QString &charSet )
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty ? sty->smoothSizes() : emptySizeList;
}


const QValueList<int> QFontDatabase::standardSizes()
{
    return QtFontStyle::standardSizes();
}

bool QFontDatabase::italic( const QString &family,
			    const QString &style,
			    const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty && sty->italic();
}

bool QFontDatabase::bold( const QString &family,
			    const QString &style,
			    const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty && sty->weight() >= QFont::Bold;
}

int QFontDatabase::weight( const QString &family,
			   const QString &style,
			   const QString &charSet ) const
{
    const QtFontStyle *sty = getStyle( d, family, style, charSet );
    return sty ? sty->weight() : -1;
}

const QStringList QFontDatabase::charSets( const QString &family,
					   bool onlyForLocale ) const
{
    const QtFontFamily *fam = d->family( family );
    return fam ? fam->charSets( onlyForLocale ) : emptyList;
}
