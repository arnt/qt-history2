/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#7 $
**
** Implementation of QFont and QFontMetrics classes for X11
**
** Author  : Eirik Eng
** Created : 940515
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#define QXFontStruct XFontStruct
#include "qfont.h"
#include "qpainter.h"

#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#7 $";
#endif


static const int exactScore   = 99999;
static const int qXFontFields = 14;

enum qXFontFieldNames   { Foundry,
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

void      qParseXFontName( char  *fontName, char **tokens );
QString   bestFitFamily  ( const  QString &s );
char    **getXFontNames  ( const  char *pattern, int *count );


class QFont_Private : public QFont
{
public:
    int     fontMatchScore( char  *fontName,      QString  buffer, 
                            float *pointSizeDiff, bool    *weightUnknown );
    QString bestMatch( QString pattern, int *score );
    QString bestFamilyMember( QString family, int *score );
    bool    findRawFont();
    QString findFont();
};

#define PRIV ((QFont_Private*)this)


void qParseXFontName( QString fontName, char **tokens )
{
    if ( fontName.isEmpty() || fontName[0] != '-' ) {
        tokens[0] = 0;
        return;
    }
    int i;
    char *tmp = fontName.data() + 1;
    for ( i = 0 ; i < qXFontFields && tmp ; i++ ) {
        tokens[i] = tmp;
        tmp = strchr( tmp, '-' );
        if( tmp )
            tmp[0] = '\0';
    }
    if ( i < qXFontFields )
        tokens[i] = 0;
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
    return family;
}

char **getXFontNames( const char *pattern, int *count )
{
    static int  maxFonts = 256;
    char **tmp;

    while( 1 ) {
        tmp = XListFonts( qXDisplay(), (char*) pattern, maxFonts, count );
        if ( *count != maxFonts || maxFonts >= 32768 )
            return tmp;
        XFreeFontNames( tmp );
        maxFonts *= 2;
    }
}

// --------------------------------------------------------------------------
// QFont dictionary to make font loading faster than using XLoadQueryFont
//

#include "qdict.h"

typedef declare(QDictM,XFontStruct) QFontDict;
typedef declare(QDictIteratorM,XFontStruct) QFontDictIt;

static QFontDict *fontDict = 0;			// dict of loaded fonts


// --------------------------------------------------------------------------
// QFont member functions
//

void QFont::initialize()			// called when starting up
{
    fontDict = new QFontDict( 29 );		// create font dictionary
    CHECK_PTR( fontDict );
}

void QFont::cleanup()				// called when terminating app
{
    Display *dpy = qXDisplay();
    QFontDictIt it( *fontDict );
    XFontStruct *f;
    while ( (f=it.current()) ) {		// free all fonts
	XFreeFont( dpy, f );
	++it;
    }
    delete fontDict;				// delete font dictionary
}


#define DEFAULT_FONT "6x13"

QFont::QFont()
{
    init();

    data->family    = "";
    data->pointSize = 12;
    data->weight    = Normal;
    data->italic    = FALSE;
    data->f         = NULL;
}

QFont::QFont( const char *family, int pointSize, int weight, bool italic )
{
    init();
    data->family    = family;
    data->pointSize = pointSize;
    data->weight    = weight;
    data->italic    = italic;
    data->f         = 0;
}


Font QFont::fontId() const
{
    if ( data->dirty )
        ((QFont*)this)->loadFont();
    return data->f->fid;
}

#undef ABS
#define ABS(X) (((X) < 0) ? (-(X)) : (X))

int QFont_Private::fontMatchScore( char  *fontName, QString buffer, 
                                   float *pointSizeDiff, bool *weightUnknown )
{
    char **tokens;
    bool   exactMatch = TRUE;
    int    score      = 0;

    strcpy( buffer.data(), fontName );    // Note! buffer must be large enough
    qParseXFontName( buffer.data(), tokens );
    
    // CharSet ###

    char pitch = tolower( tokens[Spacing][0] );
    if ( fixedPitch() ) {
        if ( pitch == 'm' || pitch == 'c' )
            score += 1000;
        else
            exactMatch = FALSE;
    } else {
        if ( pitch != 'p' )
            exactMatch = FALSE;
    }


    int   pSize = atoi( tokens[PointSize] );

    if ( strcmp( tokens[ResolutionX], "75") == 0 &&
         strcmp( tokens[ResolutionY], "75") == 0 ) {
        debug("Resolution 75x75, adding 100 to score.");
        score += 100;
    } else {
        exactMatch = FALSE;
        pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 1 ) / ( 75 * 2 );
        debug("Res = %s, adjusting pSize from %s to %i", 
              tokens[ResolutionY], tokens[PointSize], pSize );
    }


    float diff  = ( (float) ABS( pSize - pointSize() ) ) / pointSize()*100;
    if ( diff < 20 ) {
        if ( pSize != pointSize() )
            exactMatch = FALSE;
        score += 10;
        debug("diff > 20, adding 10 to score.");
    }
    if ( pointSizeDiff )
        *pointSizeDiff = diff;


    QString weightStr;//
    debug( tokens[Weight_] );
    weightStr.lower();
    *weightUnknown = FALSE;
    if ( weightStr == "medium" ) {
        if ( weight() == Normal )
            score += 2;
        else
            exactMatch = FALSE;
    } else {
        if ( weightStr == "bold" ) {
            if ( weight() == Bold )
                score += 2;
            else
                exactMatch = FALSE;
        
	} else {
            *weightUnknown = TRUE;
	}
    }

    char slant = tolower( tokens[Slant][0] );
    if ( italic() ) {
        if ( slant == 'o' || slant == 'i' )
            score += 1;
        else
            exactMatch = FALSE;
    } else {
        if ( slant == 'r' )
            score += 1;
        else
            exactMatch = FALSE;
    }
            
    return exactMatch ? exactScore : score;
}

QString QFont_Private::bestMatch( QString pattern, int *score )
{
    QString     matchBuffer( 255 );  // X never returns > 255 chars in fontname
    char **     xFontNames;
    int         count;
    int         bestScore = 0;
    char *      bestName = 0;
    float       bestPointDiff = 99;
    int         sc;
    float       pointDiff; // Difference in percent from requested point size
    bool        noWeight;  // TRUE if weight unknown

    xFontNames = getXFontNames( pattern, &count );
    for( int i = 0 ; i < count ; i++ ) {
        sc = fontMatchScore( xFontNames[i], matchBuffer, 
                             &pointDiff, &noWeight );
        debug( "%.5i %.7f [%s]", score, pointDiff, xFontNames[i] );
        if ( sc > bestScore ) {
            bestScore     = sc;
            bestName      = xFontNames[i];
            bestPointDiff = pointDiff;
	} else {
            if ( sc == bestScore ) {
                if ( pointDiff < bestPointDiff ) {
                    bestName      = xFontNames[i];
                    bestPointDiff = pointDiff;
                }
            }
        }
    }
    *score = bestScore;
    QString tmp( bestName );
    XFreeFontNames( xFontNames );
    return tmp;
}

QString QFont_Private::bestFamilyMember( QString family, int *score )
{
    QString pattern( 255 );

    pattern.sprintf("-*-%s-*-*-*-*-*-*-*-*-*-*-*-*-", family.data() );
    return bestMatch( pattern, score );
}


bool QFont_Private::findRawFont()
{
    char **fontNames;
    int    count;
    bool   success = FALSE;

    fontNames = getXFontNames( family(), &count );
    if ( fontNames[0] )
        success = TRUE;
    XFreeFontNames( fontNames );
    return success;
}

QString QFont_Private::findFont()
{
    QString     pattern( 255 );
    const char *familyName;
    QString     bestName;
    int         score;


    if ( family() == 0 || family()[0] == '\0' )
        familyName = defaultFamily();
    else
        familyName = bestFitFamily( family() );

    bestName = bestFamilyMember( familyName, &score );

    if ( score == 0 && familyName != defaultFamily() ) {
        familyName = defaultFamily();   // Try def family for style
        bestName   = bestFamilyMember( familyName, &score );        
    }

    if ( score == 0 && familyName != systemDefaultFamily() ) {
        familyName = systemDefaultFamily(); // try system default family
        bestName   = bestFamilyMember( familyName, &score );        
    }

    if ( bestName.isNull() ) { // No remotely matching fonts found
        bestName = defaultFont();
    }

}

void QFont::loadFont()
{
    QString fontName;

    if ( data->rawMode ) {
        if ( PRIV->findRawFont() )
            fontName = family();
        else
            fontName = PRIV->defaultFont();
    } else {
        fontName = PRIV->findFont(); // Returns a loadable font.
    }

    if ( !fontDict ) {
#if defined(CHECK_STATE)
	warning( "QFont::loadFont: Not ready to load font %s", name );
#endif
	return;
    }

    XFontStruct *f = fontDict->find(fontName);
    if ( !f ) {					// font is not cached
	f = XLoadQueryFont( qXDisplay(), fontName );
	if ( f )				// save for later
	    fontDict->insert( fontName, f );
	else {
            warning( "QFont::loadFont: Internal error." );   //###
	}
    }
    data->f     = f;
    data->dirty = FALSE;
    QPainter::changedFont( this, TRUE );	// tell painter about new font
}


// --------------------------------------------------------------------------
// QFontMetrics member functions
//

QFontMetrics::QFontMetrics( const QFont &font )
{
    f = font.data->f;
}

int QFontMetrics::ascent() const
{
    return f->ascent;
}

int QFontMetrics::descent() const
{
    return f->descent;
}

int QFontMetrics::height() const
{
    return f->ascent + f->descent;
}


int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    return XTextWidth( f, str, len );
}
