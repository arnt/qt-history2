/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#33 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for X11
**
** Author  : Eirik Eng
** Created : 940515
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#define QXFontStruct XFontStruct
#include "qfont.h"
#include "qfontdta.h"
#include "qfontmet.h"
#include "qfontinf.h"
#include "qpainter.h"
#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#33 $";
#endif

// #define DEBUG_FONT
static const int fontFields = 14;

enum FontFieldNames {	Foundry,
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

bool	  parseXFontName  ( QString &fontName, char **tokens );
QString	  bestFitFamily	  ( const    QString &s );
char	**getXFontNames	  ( const    char *pattern, int *count );
bool	  smoothlyScalable( const    char *fontName );
bool	  fontExists	  ( const    char *fontName );
int	  computeLineWidth( const    char *fontName );
int	  getWeight( const char *weightString, bool adjustScore = FALSE );

#define PRIV ((QFont_Private*)this)

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

bool parseXFontName( QString &fontName, char **tokens )
{
    if ( fontName.isEmpty() || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }
    int i;
    char *tmp = fontName.data() + 1;
    for ( i = 0 ; i < fontFields && tmp && tmp[0]; i++ ) {
	tokens[i] = tmp;
	tmp = strchr( tmp, '-' );
	if( tmp )
	    *tmp++ = '\0';
    }
    if ( i < fontFields ) {
	for( int j = i ; j < fontFields ; j++ )
	    tokens[j] = 0;
	return FALSE;
    }
    return TRUE;
}

QString bestFitFamily( const QString &s )
{
    QString family = s.copy();
    family.stripWhiteSpace();
    family.lower();
    if ( family == "tms rmn" ) {
	family = "times";
    } else {
	if ( family == "helv" )
	    family = "helvetica";
    }
#if defined(DEBUG_FONT)
    debug("bestFit, in = [%s], out = [%s]", s.data(), family.data() );
#endif
    return family;
}

char **getXFontNames( const char *pattern, int *count )
{
    static int	maxFonts = 256;
    char **tmp;
    while( 1 ) {
	tmp = XListFonts( qt_xdisplay(), (char*) pattern, maxFonts, count );
	if ( *count != maxFonts || maxFonts >= 32768 )
	    return tmp;
	XFreeFontNames( tmp );
	maxFonts *= 2;
    }
}

bool smoothlyScalable ( const char *fontName )
{
#if 1
    return TRUE;
#endif
}

bool fontExists( const char *fontName )
{
    char **fontNames;
    int	   count;
    fontNames	 = getXFontNames( fontName, &count );
    bool success = ( count != 0 );
    XFreeFontNames( fontNames );
    return success;
}

int computeLineWidth( const char *fontName )
{
    char *tokens[fontFields];
    QString buffer(255);   // X font name never larger than 255 chars

    strcpy( buffer, fontName );
    if ( !parseXFontName( buffer, tokens ) )
	return 1;   // Name did not conform to X Logical Font Description

    int weight = getWeight( tokens[Weight_] );
    int pSize  = atoi( tokens[PointSize] ) / 10;

    if ( strcmp( tokens[ResolutionX], "75") != 0 || // adjust if not 75 dpi
	 strcmp( tokens[ResolutionY], "75") != 0 )
	pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 75 ) / ( 75 * 2 );

		// Ad hoc algorithm, correct for bitstream charter
    int score = pSize*weight;
    int lw = ( score ) / 700;
    if ( lw < 2 && score >= 1050 ) // looks better with thicker line 
	lw = 2;			   // for small pointsizes

    return lw ? lw : 1;
}

// --------------------------------------------------------------------------
// QFont cache to make font loading faster than using XLoadQueryFont
//

#include "qcache.h"
#include "qdict.h"


struct QXFontData {
    QString	  name;
    QXFontStruct *f;
    bool dirty() { return f == 0; }
    ~QXFontData() { 
#if defined (DEBUG_FONT)
		      debug("+++ Deleting [%s]",name.data());
#endif
		      if( f ) {
			  XFreeFont( qt_xdisplay(), f );
#if defined (DEBUG_FONT)
			  debug("+++ XFreeFont [%s]",name.data());
#endif
		      }
		  }
};


declare(QCacheM,QXFontData);
typedef declare(QDictM,QXFontData) QFontDict;

static const int minCost       = 1024*100;	// overhead cost allocated when
						// increasing cache size
static const int fontCacheSize = 1024*1024*4;

class QFontCache : public QCacheM(QXFontData)
{
public:
    QFontCache(long maxCost, int size=17,bool cs=TRUE,bool ck=TRUE)
	: QCacheM(QXFontData)(maxCost,size,cs,ck){}
    void deleteItem( GCI d );
};

void QFontCache::deleteItem( GCI d ) 
{ 
    QXFontData *xfd = (QXFontData *)d;
    if ( !xfd->dirty() )
	XFreeFont( qt_xdisplay(), xfd->f );
#if defined(DEBUG)
    else
	debug("QFontCache::deleteItem: dirty delete!!!!");
#endif
    xfd->f = 0;
#if defined (DEBUG_FONT)
    debug("+++ XFreeFont [%s]",xfd->name.data());
#endif
}

struct QXFontName {
    QString	 name;
    bool	 exactMatch;
};

typedef declare(QDictM,QXFontName) QFontNameDict;

static QFontCache    *fontCache	   = 0;		// cache of loaded fonts
static QFontDict     *fontDict	   = 0;		// dict of all loaded fonts
static QFontNameDict *fontNameDict = 0;		// dict of matched font names
QFont QFont::defFont( TRUE );  // default font

// --------------------------------------------------------------------------
// QFont member functions
//

  /*!
  Returns the system default font.
  */

const QFont &QFont::defaultFont()
{
    return defFont;
}

  /*!
  Sets the system default font.
  */

void  QFont::setDefaultFont( const QFont &f )
{
    defFont = f;
}

void QFont::initialize()			// called when starting up
{
    fontCache	 = new QFontCache( fontCacheSize ); // create font cache
    CHECK_PTR( fontCache );
    fontDict	 = new QFontDict( 29 );		// create font dict
    CHECK_PTR( fontDict );
    fontNameDict = new QFontNameDict( 29 );	// create font name dictionary
    CHECK_PTR( fontNameDict );
    fontNameDict->setAutoDelete( TRUE );
}

declare (QCacheIteratorM,QXFontData);
declare (QDictIteratorM,QXFontData);

void QFont::cacheStatistics()
{
#if defined (DEBUG)
    if ( fontCache )
	fontCache->statistics();
    QCacheIteratorM(QXFontData) iter(*fontCache);
    QXFontData *tmp;
    debug("{");
    while ( (tmp = iter.current()) ) {
	debug("	  [%s]",tmp->name.data());
	++iter;
    }
    debug("}");
#endif
}

void QFont::cleanup()				// called when terminating app
{
    QDictIteratorM(QXFontData) it( *fontDict );
    QXFontData *f;
    while ( (f = it.current()) ) {		// free all fonts
	delete f;				// calls XFreeFont if necessary
	++it;
    }
    delete fontDict;
    delete fontCache;				// delete font cache
    delete fontNameDict;
}

QFont::QFont( bool )				// create default font
{
    init();
    d->req.family  = "fixed";
    d->req.rawMode = TRUE;
}
QFont::QFont()
{
    d = defFont.d;
    d->ref();
}

QFont::QFont( const char *family, int pointSize, int weight, bool italic )
{
    init();
    d->req.family    = family;
    d->req.pointSize = pointSize * 10;
    d->req.weight    = weight;
    d->req.italic    = italic;
    d->xfd	     = 0;
}

QFont::QFont( QFontData *data )			// copies a font
{
    d  = new QFontData;
    CHECK_PTR( d );
    *d = *data;
#if defined (DEBUG_FONT)
    debug("Copying font 1");
#endif
    d->count = 1;		// reset the ref count that was copied above
}

QFont QFont::copy() const
{
#if defined (DEBUG_FONT)
    debug("Copying font in copy()");
#endif
    return QFont( d );

/*    f.data->family	    = data->family;
    f.data->pointSize	  = data->pointSize;
    f.data->styleHint	  = data->styleHint;
    f.data->charSet	  = data->charSet;
    f.data->weight	  = data->weight;
    f.data->italic	  = data->italic;
    f.data->underline	  = data->underline;
    f.data->strikeOut	  = data->strikeOut;
    f.data->fixedPitch	  = data->fixedPitch;
    f.data->exactMatch	  = data->exactMatch;
    f.data->hintSetByUser = data->hintSetByUser;
    f.data->rawMode	  = data->rawMode;
    f.data->dirty	  = data->dirty;
    f.data->exactMatch	  = data->exactMatch;
    f.data->lineW	  = data->lineW;
    f.data->xfont	  = data->xfont;
    f.data->xFontName	  = data->xFontName;
*/
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->xfd is a valid pointer:

#define DIRTY_FONT ( d->req.dirty || d->xfd->dirty() )
#define DIRTY_METRICS ( f.d->req.dirty || f.d->xfd->dirty() )

Font QFont::handle() const
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

bool QFont::dirty() const
{
    return DIRTY_FONT;
}

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

QString QFont::lastResortFamily() const
{
    return "helvetica";
}

const char *tryFonts[] = { "fixed", "6x13", "9x15", 
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
			      "-*-lucida-*-*-*-*-*-*-*-*-*-*-*-*", 0 };

QString QFont::lastResortFont() const
{
    static const char *last = 0;

    if ( last )
	return last;

    const char *tmp = tryFonts[0];
    while ( tmp ) {
	if ( fontExists( tmp ) ) {
	    last = tmp;
	    return last;
	}
	tmp++;
    }
    fatal( "QFont::lastResortFont: Cannot find any reasonable font!" );
    return last;				// satisfy compiler
}


int getWeight( const char *weightString, bool adjustScore )
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
	    return (int) QFont::Bold - 1; // - 1, not sure that this IS bold
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


#undef ABS
#define ABS(X) (((X) < 0) ? (-(X)) : (X))

#define exactScore	 0xffff

#define CharSetScore	 0x40
#define PitchScore	 0x20
#define ResolutionScore	 0x10
#define SizeScore	 0x08
#define WeightScore	 0x04
#define SlantScore	 0x02
#define WidthScore	 0x01

/*
 * Returns a score describing how well a font name matches the contents
 * of a font.
 *
 */

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

    strcpy( buffer.data(), fontName );	  // Note! buffer must be large enough
    if ( !parseXFontName( buffer, tokens ) )
	return 0;   // Name did not conform to X Logical Font Description

//    debug( "parsed: [%s]",fontName );
//    for( int i = 0 ; i < fontFields ; i++ )
//	  debug( "[%s]", tokens[i] );

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
	*polymorphic = TRUE;	// Polymorphic font

    if ( IS_ZERO(tokens[PixelSize]) &&
	 IS_ZERO(tokens[PointSize]) &&
	 IS_ZERO(tokens[AverageWidth]) )
	*scalable = TRUE;    // Scalable font

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

    int	  pSize;

    if ( *scalable ) {
	pSize = deciPointSize();	  // scalable font
	score |= ResolutionScore;     // know we can ask for 75x75 resolution
    } else {
	pSize = atoi( tokens[PointSize] );
	if ( strcmp( tokens[ResolutionX], "75") == 0 &&
	     strcmp( tokens[ResolutionY], "75") == 0 ) {
#if defined(DEBUG_FONT)
	    debug("Resolution 75x75, adding 100 to score.");
#endif
	    score |= ResolutionScore;
	} else {
	    exactMatch = FALSE;
	    pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 75 ) / ( 75 * 2 );
#if defined(DEBUG_FONT)
	    debug("Res = %s, adjusting pSize from %s to %i",
		  tokens[ResolutionY], tokens[PointSize], pSize );
#endif
	}
    }

    float diff	= ( (float) ABS( pSize - deciPointSize() ) ) / deciPointSize()*100;

#if defined(DEBUG_FONT)
    debug( "pSize = %i, deciPointSize() = %i,diff = %f", pSize, deciPointSize(),diff );
#endif
    if ( diff < 20 ) {
	if ( pSize != deciPointSize() )
	    exactMatch = FALSE;
	score |= SizeScore;
#if defined(DEBUG_FONT)
	debug("diff < 20, adding 10 to score.");
#endif
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

    *weightDiff = ABS( weightVal - weight() );

#if defined(DEBUG_FONT)
    debug( "weightVal = %i, weight() = %i,diff = %i",
	    weightVal, weight(), *weightDiff );
#endif
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

    QString	matchBuffer( 255 );  // X never returns > 255 chars in fontname
    char **	xFontNames;
    int		count;
    int		sc;
    float	pointDiff;   // Difference in percent from requested point size
    int		weightDiff;  // Difference from requested weight
    bool	scalable    = FALSE;
    bool	polymorphic = FALSE;

    xFontNames = getXFontNames( pattern, &count );
    for( int i = 0 ; i < count ; i++ ) {
	sc = fontMatchScore( xFontNames[i], matchBuffer,
			     &pointDiff, &weightDiff,
			     &scalable, &polymorphic );
#if defined(DEBUG_FONT)
	debug( "%i [%s]", sc, xFontNames[i] );
#endif
	if ( sc > best.score ||
	    sc == best.score && pointDiff < best.pointDiff ||
	    sc == best.score && pointDiff == best.pointDiff &&
				weightDiff < best.weightDiff ) {
	    if ( scalable ) {
		if ( sc > bestScalable.score ||
		     sc == bestScalable.score && 
			   weightDiff < bestScalable.weightDiff ) {
#if defined(DEBUG_FONT)
		    debug( "### New best scalable score." );
#endif
		    bestScalable.score	    = sc;
		    bestScalable.name	    = xFontNames[i];
		    bestScalable.pointDiff  = pointDiff;
		    bestScalable.weightDiff = weightDiff;
		}
	    } else {
#if defined(DEBUG_FONT)
		debug( "### New best score." );
#endif
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
#if defined(DEBUG_FONT)
		debug( "*** Scaled a font: [%s]", best.name );
#endif
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
    QString pattern( 256 );
    QString match;
    pattern.sprintf("-*-%s-*-*-*-*-*-*-*-*-*-*-*-*", family.data() );
    return bestMatch( pattern, score );
}

QString QFont_Private::findFont( bool *exact )
{
    QString	familyName;
    QString	bestName;
    int		score;

    if ( family() == 0 || family()[0] == '\0' ) {
	familyName = defaultFamily();
	*exact	   = FALSE;
    } else {
	familyName = bestFitFamily( family() );
	*exact	   = TRUE;
    }
    bestName = bestFamilyMember( familyName, &score );
    if ( *exact && score != exactScore )
	*exact = FALSE;

    if ( score == 0 && familyName != defaultFamily() ) {
	familyName = defaultFamily();	// Try def family for style
	bestName   = bestFamilyMember( familyName, &score );
    }

    if ( score == 0 && familyName != lastResortFamily() ) {
	familyName = lastResortFamily(); // try system default family
	bestName   = bestFamilyMember( familyName, &score );
    }

    if ( bestName.isNull() ) { // No remotely matching fonts found
	bestName = lastResortFont();
    }
    return bestName;
}

QXFontData *loadXFont( const QString &fontName, bool *newFont )
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
#if defined(DEBUG_FONT)
	debug ("Found: [%s]\nfontName = [%s]", xfd->name.data(),
					       fontName.data() );
#endif
	if ( newFont )
	    *newFont = TRUE;	
    }
    xfd->f    = tmp;
    xfd->name = fontName;			// used by QFontInfo
#if defined(DEBUG_FONT)
    debug( "min_byte1 = %i, max_byte1 = %i", xfd->f->min_byte1,
	   xfd->f->max_byte1 );
    debug( "min_cob2 = %i, max_cob2	 = %i",
	   xfd->f->min_char_or_byte2,
	   xfd->f->max_char_or_byte2 );
#endif
    XFontStruct *f = xfd->f;
    long sz = (f->max_bounds.ascent + f->max_bounds.descent)
	      *f->max_bounds.width
	      *(f->max_char_or_byte2 - f->min_char_or_byte2) / 8;
    if ( sz > fontCache->maxCost() + minCost )
	fontCache->setMaxCost( sz + minCost ); // make room for this font only
    if ( !fontCache->insert( fontName, xfd ,sz ) ) {
#if defined(DEBUG)
	fatal("loadXFont(): internal error; cache overflow");
#endif
    }
    return xfd;
}

void QFont::loadFont() const
{
    QString instanceID;
    QXFontName	*fn;

    if ( !fontNameDict || !fontCache ) {
#if defined(CHECK_STATE)
	warning( "QFont::loadFont: Not ready to load font.");
#endif
	return;
    }
    if ( d->req.rawMode )
	instanceID = "&" + d->req.family;
    else
	instanceID.sprintf("x%s_%i_%i_%i_%i_%i_%i",
				      family(),deciPointSize(),
				      d->req.hintSetByUser ? styleHint() : 
							     QFont::AnyStyle,
				      charSet(), weight(), italic() ? 1 : 0,
				      fixedPitch() ? 1 : 0 );

#if defined(DEBUG_FONT)
    debug( "instanceID = %s", instanceID.data() );
#endif
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
	    fn->name = PRIV->findFont( &fn->exactMatch ); // Returns a 
							  // loadable font.
	}
	fontNameDict->insert( instanceID, fn );
#if defined(DEBUG_FONT)
	debug( "=== MATCHED FONT[%s] exact = %i", fn->name.data(),
						  fn->exactMatch );
#endif
    } else {
#if defined(DEBUG_FONT)
	debug( "=== DICT HIT[%s] exact = %i", fn->name.data(),
						  fn->exactMatch );
#endif
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
    d->act.dirty = TRUE;      // actual font information no longer valid
    if ( newFont )
	QPainter::changedFont( this );
#if defined(DEBUG_FONT)
    debug( "=== GOT FONT" );
#endif
}

void resetFontDef( QFontDef *def )
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

// --------------------------------------------------------------------------
// QFontMetrics member functions
//

/*! Constructs a new object that operates on \e font. \sa setFont(). */

QFontMetrics::QFontMetrics( const QFont &font )
    : f( font )
{
}

/*! Returns the maximum ascent of the font.  The ascent is the
  distance from the base line to the uppermost line where pixels may
  be drawn. */

int QFontMetrics::ascent() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->max_bounds.ascent;
}

/*! Returns the maximum descent of the font.  The descent is the
  distance from the base line to the lowermost line where pixels may
  be drawn. (Note that this is different from X, which adds 1 pixel.)
  */

int QFontMetrics::descent() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->max_bounds.descent - 1;
}

/*! Returns the height of the font.  This is always equal to
  ascent()+descent()+1 (the 1 is for the base line). \sa leading(). */

int QFontMetrics::height() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->max_bounds.ascent + f.d->xfd->f->max_bounds.descent;
}

/*! Returns the leading of the font in pixels.  This is the natural
  inter-line spacing. \sa height(). */

int QFontMetrics::leading() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->ascent		  + f.d->xfd->f->descent -
	   f.d->xfd->f->max_bounds.ascent - f.d->xfd->f->max_bounds.descent;
}

/*! Returns the distance from one base line to the next.  This value
  is always equal to leading()+height(). \sa height(), leading() */

int QFontMetrics::lineSpacing() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->ascent + f.d->xfd->f->descent;
}

/*! Returns the width in pixels of the first \e len characters of \e str.

  if \e len is negative or larger than the length of \e str, the whole
  string is used.

  Note that this value is \e not equal to boundingRect().width();
  boundingRect() returns a rectangle desribing the pixels this string
  will cover whereas width() returns the distance to where the next
  string should be drawn.  Thus, width(stra)+width(strb) is always
  equal to width(strcat(stra, strb)).  This is almost never the case
  with boundingRect(). */

int QFontMetrics::width( const char *str, int len ) const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    if ( len < 0 )
	len = strlen( str );
    return XTextWidth( f.d->xfd->f, str, len );
}

/*! Returns the bounding rectangle of the first \e len characters of \e str.

  if \e len is negative or larger than the length of \e str, the whole
  string is used.

  Note that the bounding rectangle may extend to the left of (0,0) and
  that the text output may cover \e all pixels in the bounding
  rectangle. */

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    if ( len < 0 || len > strlen(str) )
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
    if ( !f.d->act.underline && !f.d->act.strikeOut ) {
	width  = overall.rbearing - startX;
    } else {
	if ( startX > 0 )
	    startX = 0;
	if ( overall.rbearing < overall.width )
	   width =  overall.width - startX;
	else
	   width =  overall.rbearing - startX;
	if ( f.d->act.underline && len != 0 ) {
	    int ulTop = underlinePos();
	    int ulBot = ulTop + lineWidth(); // X descent is 1 
	    if ( descent < ulBot ) // more than logical descent, so don't
		descent = ulBot;   // subtract 1 here!
	    if ( ascent < -ulTop )
		ascent = -ulTop;
	}
	if ( f.d->act.strikeOut && len != 0 ) {
	    int soTop = strikeOutPos();
	    int soBot = soTop - lineWidth(); // --- "" ---
	    if ( descent < -soBot )
		descent = -soBot;
	    if ( ascent < soTop )
		ascent = soTop;
	}
    }
    return QRect( startX, -ascent, width, descent + ascent );
}

/*! Returns the width of the widest character in the font. */

int QFontMetrics::maxWidth() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->xfd->f->max_bounds.width;
}

/*! Returns the distance from the base line to where an underscore
  should be drawn. \sa strikeOutPos(), lineWidth(). */

int QFontMetrics::underlinePos() const
{
    int pos = ( lineWidth()*2 + 3 )/6; // int( ((float)lineWidth())/3 + 0.5 )
    return pos ? pos : 1;
}

/*! Returns the distance from the base line to where the strike-out
  line should be drawn. \sa underlinePos(), lineWidth(). */

int QFontMetrics::strikeOutPos() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    int pos = f.d->xfd->f->max_bounds.ascent/3;
    return pos ? pos : 1;
}

/*! Returns the width of the underline and strike-out lines, adjusted
for the point size of the font. \sa underlinePos(), strikeOutPos(). */

int QFontMetrics::lineWidth() const
{
    if ( DIRTY_METRICS )
	f.loadFont();
    return f.d->lineW;
}


// --------------------------------------------------------------------------
// QFontInfo member functions
//

QFontInfo::QFontInfo( const QFont &font )
    : f( font )
{
}

void QFont::updateFontInfo() const
{
    if ( DIRTY_FONT )
	loadFont();

    if ( exactMatch() ) { //Copy font description if matching font was found
	d->act = d->req;
	return;
    }

    char *tokens[fontFields];
    QString buffer( 255 );	 // Used to hold parsed X font name

    buffer = d->xfd->name.copy();
    if ( !parseXFontName( buffer, tokens ) ) {
#if defined(DEBUG_FONT)
	debug("QFont::updateFontInfo: Not an XLFD font name");
	debug("{%s}", d->xFontName.data() );
#endif
	resetFontDef( &d->act );
	d->exactMatch = FALSE;
	d->act.family = d->xfd->name;
	d->act.rawMode = TRUE;
	d->act.dirty   = FALSE;
	return;		 // Name did not conform to X Logical Font Description
    }

    d->act.family      = tokens[Family];
    d->act.pointSize   = atoi( tokens[PointSize] );
    d->act.styleHint   = QFont::AnyStyle;     // ###

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

    if ( strcmp( tokens[ResolutionY], "75") != 0 ) { // if not 75 dpi
	d->act.pointSize = ( 2*d->act.pointSize*atoi(tokens[ResolutionY]) + 1 )
			  / ( 75 * 2 );	 // adjust actual pointsize
    }
    d->act.underline = d->req.underline;
    d->act.strikeOut = d->req.strikeOut;
    d->act.rawMode = FALSE;
    d->act.dirty   = FALSE;
}
