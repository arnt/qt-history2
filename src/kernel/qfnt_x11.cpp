/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#52 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Author  : Eirik Eng
** Created : 940515
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwidget.h"
#include "qpainter.h"
#define GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#define QXFontStruct XFontStruct
#include "qfontdta.h"
#include "qcache.h"
#include "qdict.h"
#include <ctype.h>
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#52 $")


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

static QXFontData *loadXFont	   ( const QString &fontName, bool *newFont );
static bool	   parseXFontName  ( QString &fontName, char **tokens );
static QString	   bestFitFamily   ( const QString & );
static char	 **getXFontNames   ( const char *pattern, int *count );
static bool	   smoothlyScalable( const char *fontName );
static bool	   fontExists	   ( const char *fontName );
static int	   computeLineWidth( const char *fontName );
static int	   getWeight	   ( const char *weightString,
				     bool adjustScore = FALSE );


// QFont_Private accesses QFont protected functions

class QFont_Private : public QFont
{
public:
    int	    fontMatchScore( char  *fontName,	  QString &buffer,
			    float *pointSizeDiff, int	  *weightDiff,
			    bool  *scalable	, bool	  *polymorphic );
    QString bestMatch( const QString &pattern, int *score );
    QString bestFamilyMember( const QString &family, int *score );
    QString findFont( bool *exact );
};

#define PRIV ((QFont_Private*)this)


/*****************************************************************************
  QFont cache to make font loading faster than using XLoadQueryFont
 *****************************************************************************/

struct QXFontData {
    QString	  name;
    QXFontStruct *f;
    bool dirty() { return f == 0; }
    QXFontData() {}
   ~QXFontData() { if( f ) XFreeFont( qt_xdisplay(), f ); }
};


static const int reserveCost   = 1024*100;
static const int fontCacheSize = 1024*1024*4;


declare(QCacheM,QXFontData);			// inherited by QFontCache
typedef declare(QCacheIteratorM,QXFontData) QFontCacheIt;
typedef declare(QDictM,QXFontData)	    QFontDict;
typedef declare(QDictIteratorM,QXFontData)  QFontDictIt;


class QFontCache : public QCacheM(QXFontData)
{
public:
    QFontCache( long maxCost, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QCacheM(QXFontData)(maxCost,size,cs,ck) {}
    void deleteItem( GCI d );
};

void QFontCache::deleteItem( GCI d )
{
    QXFontData *xfd = (QXFontData *)d;
    if ( !xfd->dirty() )
	XFreeFont( qt_xdisplay(), xfd->f );
    xfd->f = 0;
}


struct QXFontName {
    QString name;
    bool    exactMatch;
};

typedef declare(QDictM,QXFontName) QFontNameDict;

static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
static QFontNameDict *fontNameDict   = 0;	// dict of matched font names
QFont		     *QFont::defFont = 0;	// default font


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

QFontData::QFontData()
{
    xfd = 0;
}

QFontData::~QFontData()
{
  // Font data is cleaned up by font cache and font dict
}

QFontData &QFontData::operator=( const QFontData &d )
{
    req = d.req;
    act = d.act;
    exactMatch = d.exactMatch;
    lineW = d.lineW;
    xfd = d.xfd;				// safe to copy
    return *this;
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Internal function that initializes the font system.
 ----------------------------------------------------------------------------*/

void QFont::initialize()			// called when starting up
{
    fontCache = new QFontCache( fontCacheSize );// create font cache
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29 );		// create font dict
    CHECK_PTR( fontDict );
    fontNameDict = new QFontNameDict( 29 );	// create font name dictionary
    CHECK_PTR( fontNameDict );
    fontNameDict->setAutoDelete( TRUE );
    if ( !defFont )
	defFont = new QFont( TRUE );		// create the default font
}

/*----------------------------------------------------------------------------
  Internal function that cleans up the font system.
 ----------------------------------------------------------------------------*/

void QFont::cleanup()				// called when terminating app
{
    fontDict->setAutoDelete( TRUE );
    delete defFont;
    delete fontDict;
    delete fontCache;				// delete font cache
    delete fontNameDict;
}

/*----------------------------------------------------------------------------
  Internal function that dumps font cache statistics.
 ----------------------------------------------------------------------------*/

void QFont::cacheStatistics()
{
#if defined(DEBUG)
    fontCache->statistics();
    QFontCacheIt it(*fontCache);
    QXFontData *tmp;
    debug( "{" );
    while ( (tmp = it.current()) ) {
	debug( "   [%s]",tmp->name.data() );
	++it;
    }
    debug( "}" );
#endif
}


/*----------------------------------------------------------------------------
  \internal
  Constructs a font object that refers to the default font.
 ----------------------------------------------------------------------------*/

QFont::QFont( bool )				// create default font
{
    init();
    d->req.family  = "fixed";
    d->req.pointSize = 12*10;			// approximate point size, hack
    d->req.weight    = QFont::Normal;
    d->req.rawMode = TRUE;
}


// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->xfd is a valid pointer:

#define DIRTY_FONT	(d->req.dirty	|| d->xfd->dirty())
#define DIRTY_METRICS	(f.d->req.dirty || f.d->xfd->dirty())


/*!
  Returns a window system handle to the font.

  Use of this function is discouraged at present.

*/

HANDLE QFont::handle( HANDLE ) const
{
    static Font last = 0;
    if ( DIRTY_FONT )
	loadFont();
    else {
	if ( d->xfd->f->fid != last ) {
	    fontCache->find( d->xfd->name );
	}
    }
    last = d->xfd->f->fid;
    return last;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the font attributes have been changed and the font has to
  be (re)loaded, or FALSE if no changes have been made.
 ----------------------------------------------------------------------------*/

bool QFont::dirty() const
{
    return DIRTY_FONT;
}


/*----------------------------------------------------------------------------
  Returns the family name that corresponds to the current style hint.
 ----------------------------------------------------------------------------*/

QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Helvetica:
	    return "helvetica";
	case Times:
	    return "times";
	case Courier:
	    return "courier";
	case Decorative:
	    return "old english";
	case System:
	    return "helvetica";
	case AnyStyle:
	default:
	    return "helvetica";
    }
}

/*----------------------------------------------------------------------------
  Returns a last resort family name for the \link fontmatch.html font
  matching algorithm. \endlink

  \sa lastResortFont()
 ----------------------------------------------------------------------------*/

QString QFont::lastResortFamily() const
{
    return "helvetica";
}

static const char *tryFonts[] = {
    "fixed",
    "6x13",
    "9x15",
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

/*----------------------------------------------------------------------------
  Returns a last resort raw font name for the \link fontmatch.html font
  matching algorithm. \endlink

  This is used if not even the last resort family is available.

  \sa lastResortFamily()
 ----------------------------------------------------------------------------*/

QString QFont::lastResortFont() const
{
    static const char *last = 0;
    if ( last )					// already found
	return last;
    const char *tmp = tryFonts[0];
    while ( tmp ) {
	if ( fontExists( tmp ) ) {
	    last = tmp;
	    return last;
	}
	tmp++;
    }
#if defined(CHECK_NULL)
    fatal( "QFont::lastResortFont: Cannot find any reasonable font!" );
#endif
    return last;
}


static void resetFontDef( QFontDef *def )	// used by updateFontInfo()
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
}

/*----------------------------------------------------------------------------
  Updates the font information.
 ----------------------------------------------------------------------------*/

void QFont::updateFontInfo() const
{
    if ( !d->act.dirty )
	return;
    if ( DIRTY_FONT )
	loadFont();
    if ( exactMatch() ) {			// match: copy font description
	d->act = d->req;
	return;
    }
    char *tokens[fontFields];
    QString buffer( 256 );			// holds parsed X font name

    buffer = d->xfd->name.copy();
    if ( !parseXFontName( buffer, tokens ) ) {
	resetFontDef( &d->act );
	d->exactMatch  = FALSE;
	d->act.family  = d->xfd->name;
	d->act.rawMode = TRUE;
	d->act.dirty   = FALSE;
	return;					// not an XLFD name
    }

    d->act.family      = tokens[Family];
    d->act.pointSize   = atoi( tokens[PointSize] );
    d->act.styleHint   = QFont::AnyStyle;	// ###

    if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 &&
	 strcmp( tokens[CharsetEncoding], "1"	    ) == 0 )
	d->act.charSet = QFont::Latin1;
    else
	d->act.charSet = QFont::AnyCharSet;

    char slant	       = tolower( tokens[Slant][0] );
    d->act.italic      = ( slant == 'o' || slant == 'i' ) ? TRUE : FALSE;

    char fixed	       = tolower( tokens[Spacing][0] );
    d->act.fixedPitch  = ( fixed == 'm' || fixed == 'c' ) ? TRUE : FALSE;

    d->act.weight      = getWeight( tokens[Weight_] );

    if ( strcmp( tokens[ResolutionY], "75") != 0 ) {	// if not 75 dpi
	d->act.pointSize = ( 2*d->act.pointSize*atoi(tokens[ResolutionY]) + 1 )
			  / ( 75 * 2 );		// adjust actual pointsize
    }
    d->act.underline = d->req.underline;
    d->act.strikeOut = d->req.strikeOut;
    d->act.rawMode = FALSE;
    d->act.dirty   = FALSE;
}


/*----------------------------------------------------------------------------
  Loads the requested font.
 ----------------------------------------------------------------------------*/

void QFont::loadFont( HANDLE ) const
{
    QString instanceID;
    QXFontName	*fn;

    if ( !fontNameDict || !fontCache )		// not initialized
	return;

    if ( d->req.rawMode )
	instanceID = "&" + d->req.family;
    else
	instanceID.sprintf( "x%s_%i_%i_%i_%i_%i_%i",
			    family(),deciPointSize(),
			    d->req.hintSetByUser ? styleHint() :
						   QFont::AnyStyle,
			    charSet(), weight(), italic() ? 1 : 0,
			    fixedPitch() ? 1 : 0 );

    fn = fontNameDict->find( instanceID );
    if ( !fn ) {
	fn = new QXFontName;
	if ( d->req.rawMode ) {
	    if ( fontExists( family() ) ) {
		fn->name       = family();
		fn->exactMatch = TRUE;
	    } else {
		fn->name       = PRIV->lastResortFont();
		fn->exactMatch = FALSE;
	    }
	} else {
	    fn->name = PRIV->findFont( &fn->exactMatch ); // returns a
							  // loadable font.
	}
	fontNameDict->insert( instanceID, fn );
    }
    bool newFont;
    d->exactMatch = fn->exactMatch;
    d->xfd	  = loadXFont( fn->name, &newFont );
    if ( !d->xfd ) {
	d->exactMatch = FALSE;
	d->xfd = loadXFont( lastResortFont(), &newFont );
	if ( !d->xfd )
	    fatal( "QFont::loadFont: Internal error" );
    }
    d->lineW	 = computeLineWidth( d->xfd->name );
    d->req.dirty = FALSE;
    d->act.dirty = TRUE;	// actual font information no longer valid
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

int QFont_Private::fontMatchScore( char	 *fontName, QString &buffer,
				   float *pointSizeDiff, int  *weightDiff,
				   bool	 *scalable     , bool *polymorphic )
{
    char *tokens[fontFields];
    bool   exactMatch = TRUE;
    int	   score      = 0;
    *scalable	   = FALSE;
    *polymorphic   = FALSE;
    *weightDiff	   = 0;
    *pointSizeDiff = 0;

    strcpy( buffer.data(), fontName );	// NOTE: buffer must be large enough
    if ( !parseXFontName( buffer, tokens ) )
	return 0;	// Name did not conform to X Logical Font Description

    if ( strncmp( tokens[CharsetRegistry], "ksc", 3 ) == 0 &&
		  isdigit( tokens[CharsetRegistry][3] )	  ||
	 strncmp( tokens[CharsetRegistry], "jisx", 4 ) == 0  &&
		  isdigit( tokens[CharsetRegistry][4] )	  ||
	 strncmp( tokens[CharsetRegistry], "gb", 2 ) == 0  &&
		  isdigit( tokens[CharsetRegistry][2] ) ) {
	     return 0; // Dirty way of avoiding common 16 bit charsets ###
    }

#undef IS_ZERO
#define IS_ZERO(X) ((strlen(X) == 1) && X[0] == '0')

    if ( IS_ZERO(tokens[Weight_]) ||
	 IS_ZERO(tokens[Slant])	  ||
	 IS_ZERO(tokens[Width]) )
	*polymorphic = TRUE;			// polymorphic font

    if ( IS_ZERO(tokens[PixelSize]) &&
	 IS_ZERO(tokens[PointSize]) &&
	 IS_ZERO(tokens[AverageWidth]) )
	*scalable = TRUE;			// scalable font

    switch( charSet() ) {
	case Latin1 :
	    if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 &&
		 strcmp( tokens[CharsetEncoding], "1"	 ) == 0 )
		score |= CharSetScore;
	    else
		exactMatch = FALSE;
	    break;
	case AnyCharSet :
	    score |= CharSetScore;
	    break;
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

    if ( *scalable ) {
	pSize = deciPointSize();	// scalable font
	score |= ResolutionScore;	// know we can ask for 75x75 resolution
    } else {
	pSize = atoi( tokens[PointSize] );
	if ( strcmp(tokens[ResolutionX], "75") == 0 &&
	     strcmp(tokens[ResolutionY], "75") == 0 ) {
	    score |= ResolutionScore;
	} else {
	    exactMatch = FALSE;
	    pSize = (2*pSize*atoi(tokens[ResolutionY]) + 75) / (75*2);
	}
    }

    float diff = ((float)QABS(pSize - deciPointSize())/deciPointSize())*100.0F;

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


QString QFont_Private::bestMatch( const QString &pattern, int *score )
{
    struct MatchData {
	MatchData(){score=0;name=0;pointDiff=99; weightDiff = 99;}
	int	score;
	char *	name;
	float	pointDiff;
	int	weightDiff;
    };

    MatchData	best;
    MatchData	bestScalable;

    QString	matchBuffer( 256 );	// X font name always <= 255 chars
    char **	xFontNames;
    int		count;
    int		sc;
    float	pointDiff;	// difference in % from requested point size
    int		weightDiff;	// difference from requested weight
    bool	scalable    = FALSE;
    bool	polymorphic = FALSE;

    xFontNames = getXFontNames( pattern, &count );
    for( int i = 0 ; i < count ; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &polymorphic );
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
    QString bestName;
    char *tokens[fontFields];

    if ( best.score != exactScore && ( bestScalable.score > best.score ||
	     bestScalable.score == best.score && (
		 best.pointDiff != 0 ||
		 best.weightDiff < bestScalable.weightDiff ) ) ) {
	if ( smoothlyScalable( bestScalable.name ) ) {
	    best.score = bestScalable.score;
	    strcpy( matchBuffer.data(), bestScalable.name );
	    if ( parseXFontName( matchBuffer, tokens ) ) {
		bestName.sprintf( "-%s-%s-%s-%s-%s-%s-*-%i-75-75-%s-*-%s-%s",
				  tokens[Foundry],
				  tokens[Family],
				  tokens[Weight_],
				  tokens[Slant],
				  tokens[Width],
				  tokens[AddStyle],
				  deciPointSize(),
				  tokens[Spacing],
				  tokens[CharsetRegistry],
				  tokens[CharsetEncoding] );
		best.name = bestName.data();
	    }
	}
    }
    *score = best.score;
    QString tmp( best.name );
    XFreeFontNames( xFontNames );
    return tmp;
}


QString QFont_Private::bestFamilyMember( const QString &family, int *score )
{
    QString pattern;
    QString match;
    pattern.sprintf( "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", family.data() );
    return bestMatch( pattern, score );
}


QString QFont_Private::findFont( bool *exact )
{
    QString familyName;
    QString bestName;
    int	    score;
    const char *fam = family();			// fam = font family name

    if ( fam == 0 || fam[0] == '\0' ) {		// null or empty string
	familyName = defaultFamily();
	*exact	   = FALSE;
    } else {
	familyName = bestFitFamily( fam );
	*exact	   = TRUE;
    }
    bestName = bestFamilyMember( familyName, &score );
    if ( *exact && score != exactScore )
	*exact = FALSE;

    if ( score == 0 ) {
	QString df = defaultFamily();
	if( familyName != df ) {
	    familyName = df;			// try default family for style
	    bestName   = bestFamilyMember( familyName, &score );
	}
    }
    if ( score == 0 ) {
	QString lrf = lastResortFamily();
	if ( familyName != lrf ) {
	    familyName = lrf;			// try system default family
	    bestName   = bestFamilyMember( familyName, &score );
	}
    }
    if ( bestName.isNull() ) {			// no matching fonts found
	bestName = lastResortFont();
    }
    return bestName;
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Returns the maximum ascent of the font.

  The ascent is the distance from the base line to the uppermost line
  where pixels may be drawn.

  \sa descent()
 ----------------------------------------------------------------------------*/

int QFontMetrics::ascent() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->xfd->f->max_bounds.ascent;
}


/*----------------------------------------------------------------------------
  Returns the maximum descent of the font.

  The descent is the distance from the base line to the lowermost line
  where pixels may be drawn. (Note that this is different from X, which
  adds 1 pixel.)

  \sa ascent()
 ----------------------------------------------------------------------------*/

int QFontMetrics::descent() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->xfd->f->max_bounds.descent - 1;
}


/*----------------------------------------------------------------------------
  Returns the height of the font.

  This is always equal to ascent()+descent()+1 (the 1 is for the base line).

  \sa leading(), lineSpacing()
 ----------------------------------------------------------------------------*/

int QFontMetrics::height() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->xfd->f->max_bounds.ascent + f.d->xfd->f->max_bounds.descent;
}


/*----------------------------------------------------------------------------
  Returns the leading of the font.

  This is the natural inter-line spacing.

  \sa height(), lineSpacing()
 ----------------------------------------------------------------------------*/

int QFontMetrics::leading() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    int l = f.d->xfd->f->ascent		  + f.d->xfd->f->descent -
	   f.d->xfd->f->max_bounds.ascent - f.d->xfd->f->max_bounds.descent;
    if ( l > 0 )
	return l;
    else
	return 0;
}


/*----------------------------------------------------------------------------
  Returns the distance from one base line to the next.

  This value is always equal to leading()+height().
  \sa height(), leading()
 ----------------------------------------------------------------------------*/

int QFontMetrics::lineSpacing() const
{
    /*
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->xfd->f->ascent + f.d->xfd->f->descent;
    */
    return leading()+height();
}


/*----------------------------------------------------------------------------
  Returns the width in pixels of the first \e len characters of \e str.

  If \e len is negative (default value), the whole string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle describing the pixels this string
  will cover whereas width() returns the distance to where the next string
  should be drawn.  Thus, width(stra)+width(strb) is always equal to
  width(strcat(stra, strb)).  This is almost never the case with
  boundingRect().

  \sa boundingRect()
 ----------------------------------------------------------------------------*/

int QFontMetrics::width( const char *str, int len ) const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    if ( len < 0 )
	len = strlen( str );
    return XTextWidth( f.d->xfd->f, str, len );
}


/*----------------------------------------------------------------------------
  Returns the bounding rectangle of the first \e len characters of \e str.

  If \e len is negative (default value), the whole string is used.

  Note that the bounding rectangle may extend to the left of (0,0) and
  that the text output may cover \e all pixels in the bounding rectangle.

  \sa width()
 ----------------------------------------------------------------------------*/

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    if ( len < 0 )
	len = strlen( str );
    XFontStruct *fs = f.d->xfd->f;
    int direction;
    int ascent;
    int descent;
    XCharStruct overall;

    XTextExtents( fs, str, len, &direction, &ascent, &descent, &overall );
    int startX = overall.lbearing;
    int width  = overall.rbearing - startX;
    ascent     = overall.ascent;
    descent    = overall.descent;
    if ( !f.d->req.underline && !f.d->req.strikeOut ) {
	width  = overall.rbearing - startX;
    } else {
	if ( startX > 0 )
	    startX = 0;
	if ( overall.rbearing < overall.width )
	   width =  overall.width - startX;
	else
	   width =  overall.rbearing - startX;
	if ( f.d->req.underline && len != 0 ) {
	    int ulTop = underlinePos();
	    int ulBot = ulTop + lineWidth(); // X descent is 1
	    if ( descent < ulBot )	// more than logical descent, so don't
		descent = ulBot;	// subtract 1 here!
	    if ( ascent < -ulTop )
		ascent = -ulTop;
	}
	if ( f.d->req.strikeOut && len != 0 ) {
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


/*----------------------------------------------------------------------------
  Returns the width of the widest character in the font.
 ----------------------------------------------------------------------------*/

int QFontMetrics::maxWidth() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->xfd->f->max_bounds.width;
}


/*----------------------------------------------------------------------------
  Returns the distance from the base line to where an underscore should be
  drawn.
  \sa strikeOutPos(), lineWidth()
 ----------------------------------------------------------------------------*/

int QFontMetrics::underlinePos() const
{
    int pos = ( lineWidth()*2 + 3 )/6;
    return pos ? pos : 1;
}


/*----------------------------------------------------------------------------
  Returns the distance from the base line to where the strike-out line
  should be drawn.
  \sa underlinePos(), lineWidth()
 ----------------------------------------------------------------------------*/

int QFontMetrics::strikeOutPos() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    int pos = f.d->xfd->f->max_bounds.ascent/3;
    return pos ? pos : 1;
}


/*----------------------------------------------------------------------------
  Returns the width of the underline and strike-out lines, adjusted for
  the point size of the font.
  \sa underlinePos(), strikeOutPos()
 ----------------------------------------------------------------------------*/

int QFontMetrics::lineWidth() const
{
    QFont f = data.widget ? data.w->font() : data.p->font();
    f.handle();
    return f.d->lineW;
}


/*****************************************************************************
  Internal X font functions
 *****************************************************************************/

//
// Loads an X font
//

static QXFontData *loadXFont( const QString &fontName, bool *newFont )
{
    if ( !fontCache )
	return 0;

    if ( newFont )
	*newFont = FALSE;
    QXFontData *xfd = fontCache->find( fontName );
    if ( xfd )
	return xfd;
						// font is not cached
    QXFontStruct  *tmp;
    tmp = XLoadQueryFont( qt_xdisplay(), fontName );
    if ( !tmp )
	return 0;				// could not load font

    xfd = fontDict->find( fontName );
    if ( !xfd ) {				// first time font is loaded?
	xfd = new QXFontData;
	CHECK_PTR( xfd );
	fontDict->insert( fontName, xfd );
    } else {
	if ( newFont )
	    *newFont = TRUE;
    }
    xfd->f    = tmp;
    xfd->name = fontName;			// used by QFontInfo
    XFontStruct *f = xfd->f;
    long sz = (f->max_bounds.ascent + f->max_bounds.descent)
	      *f->max_bounds.width
	      *(f->max_char_or_byte2 - f->min_char_or_byte2) / 8;
    if ( sz > fontCache->maxCost() + reserveCost )	// make room for this
	fontCache->setMaxCost( sz + reserveCost );	//   font only
    if ( !fontCache->insert( fontName, xfd ,sz ) ) {
#if defined(DEBUG)
	fatal( "loadXFont(): internal error; cache overflow" );
#endif
    }
    return xfd;
}


//
// Splits an X font name into fields separated by '-'
//

static bool parseXFontName( QString &fontName, char **tokens )
{
    if ( fontName.isEmpty() || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }
    int	  i;
    char *tmp = fontName.data() + 1;
    for ( i=0; i<fontFields && tmp && tmp[0]; i++ ) {
	tokens[i] = tmp;
	tmp = strchr( tmp, '-' );
	if( tmp )
	    *tmp++ = '\0';
    }
    if ( i < fontFields ) {
	for( int j=i ; j<fontFields; j++ )
	    tokens[j] = 0;
	return FALSE;
    }
    return TRUE;
}


//
// Maps a font family name to a valid X family name
//

static QString bestFitFamily( const QString &fam )
{
    QString family = QFont::substitute( fam );
    family.lower();
    return family;
}


//
// Get an array of X font names that matches a pattern
//

static char **getXFontNames( const char *pattern, int *count )
{
    static int maxFonts = 256;
    char **tmp;
    while( 1 ) {
	tmp = XListFonts( qt_xdisplay(), (char*) pattern, maxFonts, count );
	if ( *count != maxFonts || maxFonts >= 32768 )
	    return tmp;
	XFreeFontNames( tmp );
	maxFonts *= 2;
    }
}


//
// Returns TRUE if the font can be smoothly scaled
//

static bool smoothlyScalable ( const char * /* fontName */  )
{
    return TRUE;
}


//
// Returns TRUE if the font exists, FALSE otherwise
//

static bool fontExists( const char *fontName )
{
    char **fontNames;
    int	   count;
    fontNames	 = getXFontNames( fontName, &count );
    bool success = ( count != 0 );
    XFreeFontNames( fontNames );
    return success;
}


//
// Computes the line width (underline,strikeout) for the X font.
//

static int computeLineWidth( const char *fontName )
{
    char *tokens[fontFields];
    QString buffer(256);		// X font name always <= 255 chars
    strcpy( buffer.data(), fontName );
    if ( !parseXFontName(buffer, tokens) )
	return 1;			// name did not conform to X LFD
    int weight = getWeight( tokens[Weight_] );
    int pSize  = atoi( tokens[PointSize] ) / 10;
    if ( strcmp( tokens[ResolutionX], "75") != 0 || // adjust if not 75 dpi
	 strcmp( tokens[ResolutionY], "75") != 0 )
	pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 75 ) / ( 75 * 2 );
    int score = pSize*weight;		// ad hoc algorithm
    int lw = ( score ) / 700;
    if ( lw < 2 && score >= 1050 )	// looks better with thicker line
	lw = 2;				//   for small pointsizes
    return lw ? lw : 1;
}


//
// Converts a weight string to a value
//

static int getWeight( const char *weightString, bool adjustScore )
{
    QString wStr = weightString;
    wStr.lower();
    if ( wStr == "medium" )
	return (int) QFont::Normal;
    if ( wStr == "bold" )
	return (int) QFont::Bold;
    if ( wStr == "demibold" )
	return (int) QFont::DemiBold;
    if ( wStr == "black" )
	return (int) QFont::Black;
    if ( wStr == "light" )
	return (int) QFont::Light;
    if ( wStr.contains( "bold" ) ) {
	if( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
       else
	    return (int) QFont::Bold;
    }
    if ( wStr.contains( "light" ) ) {
	if( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
       else
	    return (int) QFont::Light;
    }
    if ( wStr.contains( "black" ) ) {
	if( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
       else
	    return (int) QFont::Black;
    }
    if ( adjustScore )
	return (int) QFont::Normal - 2; // - 2, we hope it's close to normal
    else
	return (int) QFont::Normal;
}
