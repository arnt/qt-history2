/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#13 $
**
** Implementation of QFont and QFontMetrics classes for X11
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
#include "qpainter.h"
#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_x11.cpp#13 $";
#endif


#define DEBUG_FONT
static const int fontFields = 14;

enum FontFieldNames {   Foundry,                // !!!hanord: Enum har stor forbokstav, f.eks. FontFieldNames (qX.. er for globale funksjoner)
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

bool      qParseXFontName ( QString &fontName, char **tokens ); // !!!hanord: parseXFontName
QString   bestFitFamily   ( const  QString &s );
char    **getXFontNames   ( const  char *pattern, int *count );
bool      smoothlyScalable( const char *fontName );


class QFont_Private : public QFont
{
public:
    int     fontMatchScore( char  *fontName,      QString &buffer,
                            float *pointSizeDiff, bool    *weightUnknown,
                            bool  *scalable     , bool *polymorphic );
    QString bestMatch( const QString &pattern, int  *score );
    QString bestFamilyMember( const QString &family, int *score );
    bool    findRawFont();
    QString findFont();
};

#define PRIV ((QFont_Private*)this)


bool qParseXFontName( QString &fontName, char **tokens )
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

bool smoothlyScalable ( const char *fontName )
{
#if 1
    return TRUE;
#else
    int            count;
    XFontStruct  **info;
    char         **fontNames;
    char          *value;
    bool           scalable = FALSE;
    Atom           xa_font_type;

#if defined(DEBUG_FONT)
    debug( "### smoothly: %s", fontName );
#endif
    fontNames = XListFontsWithInfo( qXDisplay(), fontName, 1, &count, info );
    if ( fontNames && count > 0 ) { // Font found ?
        xa_font_type = XInternAtom( qXDisplay(), "FONT_TYPE", TRUE );
        if ( xa_font_type != None ) { // atom found?
            if ( !XGetFontProperty( info[0], xa_font_type,
                       (unsigned long *) &value ) ) {//property found?
#if defined(DEBUG_FONT)
                debug( " Got it, and its called: [%s]", value );
#endif
                QString fontType = value;
                fontType.lower();   // Convert to lowercase
                if ( fontType != "bitmap" && fontType != "prebuildt" )
                    scalable =  TRUE; // Font type defined and is not bitmap
            }
        }
    }
//    XFreeFontInfo( fontNames, *info, count );
//    XFreeFontNames( fontNames );
    return scalable;
#endif
}

// --------------------------------------------------------------------------
// QFont dictionary to make font loading faster than using XLoadQueryFont
//

#include "qdict.h"

struct QXFontData {
    QXFontData() {}
    QXFontData( const char *n, XFontStruct *f ) { name=n; font=f; }
    QString      name;
    XFontStruct *font;
};

typedef declare(QDictM,QXFontData) QFontDict;
typedef declare(QDictIteratorM,QXFontData) QFontDictIt;

static QFontDict *fontDict = 0;                 // dict of loaded fonts
static QFontDict *fontNameDict = 0;             // dict of matched font names


// --------------------------------------------------------------------------
// QFont member functions
//

void QFont::initialize()                        // called when starting up
{
    fontDict     = new QFontDict( 29 );         // create font dictionary
    CHECK_PTR( fontDict );
    fontNameDict = new QFontDict( 29 );         // create font name dictionary
    CHECK_PTR( fontNameDict );
}

void QFont::cleanup()                           // called when terminating app
{
    Display *dpy = qXDisplay();
    QFontDictIt it( *fontDict );
    QXFontData *f;
    while ( ( f = it.current() ) ) {            // free all fonts
        XFreeFont( dpy, f->font );
        delete f;
        ++it;
    }
    delete fontDict;                            // delete font dictionary
    delete fontNameDict;
}

#define DEFAULT_FONT "6x13"

QFont::QFont()
{
    init();
    data->family    = "";
    data->pointSize = 120;
    data->weight    = Normal;
    data->italic    = FALSE;
    data->f         = 0;
}

QFont::QFont( const char *family, int pointSize, int weight, bool italic )
{
    init();
    data->family    = family;
    data->pointSize = pointSize * 10;
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


int getWeight( const char *weightString, bool adjustScore = FALSE )
{
    QString wStr = weightString;
    wStr.lower();

    if ( wStr == "medium" )
        return (int) QFont::Normal;

    if ( wStr == "bold" )
        return (int) QFont::Bold;

    if ( wStr == "demibold" )
        return (int) QFont::Bold - 12;

    if ( wStr == "black" )
        return (int) QFont::Bold + 12;

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
            return (int) QFont::Bold + 12 - 1; // - 1, not sure this IS black
       else
            return (int) QFont::Bold + 12;
    }

    if ( adjustScore )
        return (int) QFont::Normal - 2; // - 2, we hope it's close to normal
    else
        return (int) QFont::Normal;
}


#undef ABS
#define ABS(X) (((X) < 0) ? (-(X)) : (X))

#define exactScore       0xff

#define CharSetScore     0x20
#define PitchScore       0x10
#define ResolutionScore  0x08
#define SizeScore        0x04
#define WeightScore      0x02
#define SlantScore       0x01


/*
 * Returns a score describing how well a font name matches the contents
 * of a font.
 *
 */

int QFont_Private::fontMatchScore( char  *fontName, QString &buffer,
                                   float *pointSizeDiff, int  *weightDiff,
                                   bool  *scalable     , bool *polymorphic )
{
    char *tokens[fontFields];
    bool   exactMatch = TRUE;
    int    score      = 0;
    *scalable      = FALSE;
    *polymorphic   = FALSE;
    *weightDiff    = 0;
    *pointSizeDiff = 0;

    strcpy( buffer.data(), fontName );    // Note! buffer must be large enough
    if ( !qParseXFontName( buffer, tokens ) )
        return 0;   // Name did not conform to X Logical Font Description

//    debug( "parsed: [%s]",fontName );
//    for( int i = 0 ; i < fontFields ; i++ )
//        debug( "[%s]", tokens[i] );

    if ( strncmp( tokens[CharsetRegistry], "ksc", 3 ) == 0 &&
                  isdigit( tokens[CharsetRegistry][3] )   ||
         strncmp( tokens[CharsetRegistry], "jisx", 4 ) == 0  &&
                  isdigit( tokens[CharsetRegistry][4] )   ||
         strncmp( tokens[CharsetRegistry], "gb", 2 ) == 0  &&
                  isdigit( tokens[CharsetRegistry][2] ) ) {
             return 0; // Dirty way of avoiding common 16 bit charsets ###
    }

#undef IS_ZERO
#define IS_ZERO(X) ((strlen(X) == 1) && X[0] == '0')

    if ( IS_ZERO(tokens[Weight_]) ||
         IS_ZERO(tokens[Slant])   ||
         IS_ZERO(tokens[Width]) )
        *polymorphic = TRUE;    // Polymorphic font

    if ( IS_ZERO(tokens[PixelSize]) &&
         IS_ZERO(tokens[PointSize]) &&
         IS_ZERO(tokens[AverageWidth]) )
        *scalable = TRUE;    // Scalable font

    switch( charSet() ) {
        case Latin1 :
                 if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 &&
                      strcmp( tokens[CharsetEncoding], "1"       ) == 0 )
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

    int   pSize;

    if ( *scalable ) {
        pSize = deciPointSize();          // scalable font
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
            pSize = ( 2*pSize*atoi(tokens[ResolutionY]) + 1 ) / ( 75 * 2 );
#if defined(DEBUG_FONT)
            debug("Res = %s, adjusting pSize from %s to %i",
                  tokens[ResolutionY], tokens[PointSize], pSize );
#endif
        }
    }

    float diff  = ( (float) ABS( pSize - deciPointSize() ) ) / deciPointSize()*100;

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
    return exactMatch ? exactScore : score;
}

QString QFont_Private::bestMatch( const QString &pattern, int *score )
{
    struct MatchData {
        MatchData(){score=0;name=0;pointDiff=99; weightDiff = 99;}
        int     score;
        char *  name;
        float   pointDiff;
        int     weightDiff;
    };

    MatchData   best;
    MatchData   bestScalable;

    QString     matchBuffer( 255 );  // X never returns > 255 chars in fontname
    char **     xFontNames;
    int         count;
    int         sc;
    float       pointDiff;   // Difference in percent from requested point size
    int         weightDiff;  // Difference from requested weight
    bool        scalable    = FALSE;
    bool        polymorphic = FALSE;

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
                    bestScalable.score      = sc;
                    bestScalable.name       = xFontNames[i];
                    bestScalable.pointDiff  = pointDiff;
                    bestScalable.weightDiff = weightDiff;
                }
            } else {
#if defined(DEBUG_FONT)
                debug( "### New best score." );
#endif
                best.score      = sc;
                best.name       = xFontNames[i];
                best.pointDiff  = pointDiff;
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
            if ( qParseXFontName( matchBuffer, tokens ) ) {
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
    QString     familyName;
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
    return bestName;
}



QXFontData *loadXFont( const char *fontName )
{
    QXFontData  *fd;
    XFontStruct *tmp;

    if ( !fontDict )
        return 0;

    fd = fontDict->find( fontName );
    if ( fd )
        return fd;
                                          // font is not cached
    tmp = XLoadQueryFont( qXDisplay(), fontName );
    if ( !tmp )
        return 0;                         // could not load font

    fd = new QXFontData;
    CHECK_PTR( fd );
    fd->font = tmp;
    fd->name = fontName;                  // used by QFontMetrics
#if defined(DEBUG_FONT)
    debug( "min_byte1 = %i, max_byte1 = %i", fd->font->min_byte1,
           fd->font->max_byte1 );
    debug( "min_cob2 = %i, max_cob2      = %i",
           fd->font->min_char_or_byte2,
           fd->font->max_char_or_byte2 );
#endif
    fontDict->insert( fontName, fd );

}

void QFont::loadFont() const
{
/************************************************************
    QString fontName;
    QString instanceID;
    QXFontData  *fd;

    if ( !fontNameDict || !fontDict ) {
#if defined(CHECK_STATE)
        warning( "QFont::loadFont: Not ready to load font.");
#endif
        return;
    }
    if ( data->rawMode )
        instanceID = "&" + data->family;
    else
        instanceID.sprintf("%s_%i_%i_%i_%i_%i_%i",
                                      family(),deciPointSize(),
                                      data->hintSetByUser ? styleHint : 0,
                                      charSet(), weight(), italic() ? 1 : 0,
                                      fixedPitch() ? 1 : 0 );

#if defined(DEBUG_FONT)
    debug( "instanceID = %s", instanceID.data() );
#endif
    fd = fontNameDict->find( instanceID );

    if ( !fd ) {
        if ( data->rawMode ) {
/*            if ( PRIV->findRawFont() ) {
                fd = loadXFont( family() ) ;
                fontName = family();
            } else {
                fontName = PRIV->defaultFont();
            }
            fd = loadXFont( fontName );
            bestName = PRIV->bestFamilyMember( familyName, &score ); * /
        } else {
        QString familyName;
        QString bestName;
        int     score;

        if ( data->family.isEmpty() )
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
    return bestName;




        }
    fontNameDict->insert( instanceID, fd );
    }
    data->xFontName = fd->name;
    data->f         = fd->font;
    data->dirty     = FALSE;

***********************************************************/


    QString fontName;
    QString instanceID;
    QXFontData	*fd;

    if ( !fontNameDict ) {
#if defined(CHECK_STATE)
	warning( "QFont::loadFont: Not ready to load font.");
#endif
	return;
    }
    if ( data->rawMode )
	instanceID = "&" + data->family;
    else
	instanceID.sprintf("%s_%i_%i_%i_%i_%i_%i",
				      family(),deciPointSize(),
				      data->hintSetByUser ? styleHint : 0,
				      charSet(), weight(), italic() ? 1 : 0,
				      fixedPitch() ? 1 : 0 );

#if defined(DEBUG_FONT)
    debug( "instanceID = %s", instanceID.data() );
#endif
    fd = fontNameDict->find( instanceID );
    if ( !fd ) {
	if ( data->rawMode ) {
	    if ( PRIV->findRawFont() )
		fontName = family();
	    else
		fontName = PRIV->defaultFont();
	} else {
	    fontName = PRIV->findFont(); // Returns a loadable font.
	}
#if defined(DEBUG_FONT)
	debug( "=== MATCHED FONT[%s] ", fontName.data() );
#endif
	if ( !fontDict ) {
#if defined(CHECK_STATE)
	    warning( "QFont::loadFont: Not ready to load font %s");
#endif
	    return;
	}
	fd = fontDict->find(fontName);
	if ( !fd ) {					// font is not cached
	    fd = new QXFontData;
	    CHECK_PTR( fd );
	    fd->name = fontName;
	    fd->font = XLoadQueryFont( qXDisplay(), fontName );
	    if ( fd->font ) {				// save for later
#if defined(DEBUG_FONT)
		debug( "min_byte1 = %i, max_byte1 = i", fd->font->min_byte1,
							fd->font->max_byte1 );
		debug( "min_cob2 = %i, max_cob2	  = i",
		       fd->font->min_char_or_byte2,
		       fd->font->max_char_or_byte2 );
#endif
		fontDict->insert( fontName, fd );
	    } else {
		warning( "QFont::loadFont: PANIC! Could not load X font!." );
		delete fd;
		return;
	    }
#if defined(DEBUG_FONT)
	    debug( "=== LOADED FONT" );
#endif
	}
	fontNameDict->insert( instanceID, fd );
    }
    data->xFontName = fd->name;

//    qat *tmp = fet;

    data->f	    = fd->font;
    data->dirty	    = FALSE;
#if defined(DEBUG_FONT)
    debug( "=== GOT FONT" );
#endif



}


// --------------------------------------------------------------------------
// QFontMetrics member functions
//


void resetFontData( QFontData *data )
{
    data->family        = "";
    data->pointSize     = 0;
    data->weight        = QFont::AnyWeight;
    data->italic        = FALSE;
    data->styleHint     = QFont::AnyStyle;
    data->charSet       = QFont::Latin1;
    data->fixedPitch    = FALSE;
    data->dirty         = TRUE;
    data->exactMatch    = FALSE;
    data->hintSetByUser = FALSE;
    data->rawMode       = FALSE;
    data->f             = 0;
}


QFontMetrics::QFontMetrics()
{
    f    = 0;
    data = new QFontData;
    CHECK_PTR( data );
    data->dirty = TRUE;
}

QFontMetrics::QFontMetrics( const QFont &font )
{
    f    = &font;
    data = new QFontData;
    CHECK_PTR( data );
    data->dirty = TRUE;
}

QFontMetrics::~QFontMetrics()
{
    if ( data->deref() )
	delete data;
}

int QFontMetrics::ascent() const
{
    if ( !f )
        return 0;
    if ( f->data->dirty )
        f->loadFont();
    return f->data->f->ascent;
}

int QFontMetrics::descent() const
{
    if ( !f )
        return 0;
    if ( f->data->dirty )
        f->loadFont();
    return f->data->f->descent;
}

int QFontMetrics::height() const
{
    if ( !f )
        return 0;
    if ( f->data->dirty )
        f->loadFont();
    return f->data->f->ascent + f->data->f->descent;
}


int QFontMetrics::width( const char *str, int len ) const
{
    if ( !f )
        return 0;
    if ( f->data->dirty )
        f->loadFont();
    if ( len < 0 )
        len = strlen( str );
    return XTextWidth( f->data->f, str, len );
}

void QFontMetrics::updateData( ) const
{
    if ( !f )
        return;

    if ( f->data->dirty )
        f->loadFont();

//    if ( f->rawMode )  ### Load font info here to fill inn QFontData fields!

    if ( f->exactMatch() ) { //Copy font description if matching font was found
        *data = *(f->data);
        return;
    }

    char *tokens[fontFields];
    QString buffer( 255 );       // Used to hold parsed X font name

    buffer = f->data->xFontName.copy();
    if ( !qParseXFontName( buffer, tokens ) ) {
#if defined(DEBUG_FONT)
        debug("QFontMetrics::updateData: Internal error, reseting font data");
        debug("{%s}", f->data->xFontName.data() );
#endif
        resetFontData( data );
        return;          // Name did not conform to X Logical Font Description
    }

    data->family      = tokens[Family];
    data->pointSize   = atoi( tokens[PointSize] );
    data->styleHint   = QFont::AnyStyle;     // ###

    if ( strcmp( tokens[CharsetRegistry], "iso8859" ) == 0 &&
         strcmp( tokens[CharsetEncoding], "1"       ) == 0 )
        data->charSet     = QFont::Latin1;
    else
        data->charSet     = QFont::AnyCharSet;

    char slant        = tolower( tokens[Slant][0] );
    data->italic      = ( slant == 'o' || slant == 'i' ) ? TRUE : FALSE;

    char fixed        = tolower( tokens[Spacing][0] );
    data->fixedPitch  = ( fixed == 'm' || fixed == 'c' ) ? TRUE : FALSE;

    data->exactMatch  = f->data->exactMatch;
    data->f           = f->data->f;

    data->weight      = getWeight( tokens[Weight_] );

    if ( strcmp( tokens[ResolutionY], "75") != 0 ) { // If not 75 dpi
        data->pointSize = ( 2*data->pointSize*atoi(tokens[ResolutionY]) + 1 )
                          / ( 75 * 2 );  // Adjust actual pointsize
    }
}
