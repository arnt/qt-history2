/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_x11.cpp#9 $
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
#include "qfontdta.h"
#include "qpainter.h"

#include <ctype.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont_x11.cpp#9 $";
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

bool      qParseXFontName ( QString &fontName, char **tokens );
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
    for ( i = 0 ; i < qXFontFields && tmp && tmp[0]; i++ ) {
        tokens[i] = tmp;
        tmp = strchr( tmp, '-' );
        if( tmp )
            *tmp++ = '\0';
    }
    if ( i < qXFontFields ) {
        for( int j = i ; j < qXFontFields ; j++ )
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
    debug("bestFit, in = [%s], out = [%s]", s.data(), family.data() );

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

#include <X11/Xatom.h>

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

    debug( "### smoothly: %s", fontName );

    fontNames = XListFontsWithInfo( qXDisplay(), fontName, 1, &count, info );
    if ( fontNames && count > 0 ) { // Font found ?
        xa_font_type = XInternAtom( qXDisplay(), "FONT_TYPE", TRUE );
        if ( xa_font_type != None ) { // atom found?
            if ( !XGetFontProperty( info[0], xa_font_type,
                       (unsigned long *) &value ) ) {//property found?
                debug( " Got it, and its called: [%s]", value );
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
    QXFontData(){}
    QXFontData( const char *nm, XFontStruct *f ){name=nm;font=f;}
    QString      name;
    XFontStruct *font;
};

typedef declare(QDictM,QXFontData) QFontDict;
typedef declare(QDictIteratorM,QXFontData) QFontDictIt;

static QFontDict *fontDict = 0;			// dict of loaded fonts
static QFontDict *fontNameDict = 0;		// dict of matched font names


// --------------------------------------------------------------------------
// QFont member functions
//

void QFont::initialize()			// called when starting up
{
    fontDict     = new QFontDict( 29 );		// create font dictionary
    CHECK_PTR( fontDict );
    fontNameDict = new QFontDict( 29 );		// create font name dictionary
    CHECK_PTR( fontNameDict );
}

void QFont::cleanup()				// called when terminating app
{
    Display *dpy = qXDisplay();
    QFontDictIt it( *fontDict );
    QXFontData *f;
    while ( ( f = it.current() ) ) {		// free all fonts
	XFreeFont( dpy, f->font );
        delete f;
	++it;
    }
    delete fontDict;				// delete font dictionary
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


int getWeight( const char *weightString )
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

    if ( wStr.contains( "bold" ) )
        return (int) QFont::Bold - 1;      // - 1, not sure that this IS bold

    if ( wStr.contains( "light" ) )
        return (int) QFont::Light - 1;     // - 1, not sure that this IS light

    if ( wStr.contains( "black" ) )
        return (int) QFont::Bold + 12 - 1; // - 1, not sure that this IS black

    return (int) QFont::Normal - 2;        // - 2, we hope it's close to normal
}


#undef ABS
#define ABS(X) (((X) < 0) ? (-(X)) : (X))


/*
 * Returns a score describing how well a font name matches the contents
 * of a font.
 *
 */

int QFont_Private::fontMatchScore( char  *fontName, QString &buffer, 
                                   float *pointSizeDiff, int  *weightDiff,
                                   bool  *scalable     , bool *polymorphic )
{
    char *tokens[qXFontFields];
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
//    for( int i = 0 ; i < qXFontFields ; i++ )
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
                     score += 10000;
                 else
                     exactMatch = FALSE;
                 break;
        case AnyCharSet :
                     score += 10000;
                 break;
    }

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

    int   pSize;
    
    if ( *scalable ) {
        pSize = pointSize(); // scalable font
        score += 100;        // know we can ask for 75x75 resolution
    } else {
        pSize = atoi( tokens[PointSize] );

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
    }

    float diff  = ( (float) ABS( pSize - pointSize() ) ) / pointSize()*100;

    debug( "pSize = %i, pointSize() = %i,diff = %f", pSize, pointSize(),diff );

    if ( diff < 20 ) {
        if ( pSize != pointSize() )
            exactMatch = FALSE;
        score += 10;
        debug("diff < 20, adding 10 to score.");
    } else {
        exactMatch = FALSE;
   }
    if ( pointSizeDiff )
        *pointSizeDiff = diff;


    int weightVal = getWeight( tokens[Weight_] );

    if ( weightVal == weight() )
        score += 2;
    else
        exactMatch = FALSE;

    *weightDiff = ABS( weightVal - weight() );

    debug( "weightVal = %i, weight() = %i,diff = %i", 
            weightVal, weight(), *weightDiff );

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

QString QFont_Private::bestMatch( const QString &pattern, int *score )
{
    struct MatchData {
        MatchData(){score=0;name=0;pointDiff=99; weightDiff = 99;}
        int	score;
        char *	name;
        float	pointDiff;
        int	weightDiff;
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
        debug( "%i [%s]", sc, xFontNames[i] );
        if ( sc > best.score || 
            sc == best.score && pointDiff < best.pointDiff ||
            sc == best.score && pointDiff == best.pointDiff && 
                                weightDiff < best.weightDiff ) {
            if ( scalable ) {
                if ( sc > bestScalable.score ) {
                    debug( "### New best scalable score." );
                    bestScalable.score      = sc;
                    bestScalable.name       = xFontNames[i];
                    bestScalable.pointDiff  = pointDiff;
                    bestScalable.weightDiff = weightDiff;
                }
	    } else {
                debug( "### New best score." );
                best.score      = sc;
                best.name       = xFontNames[i];
                best.pointDiff  = pointDiff;
                best.weightDiff = weightDiff;
	    }
	}
    }
    QString bestName;
    char *tokens[qXFontFields];
    if ( best.score != exactScore && bestScalable.score > best.score ) {
        if ( smoothlyScalable( bestScalable.name ) ) {

            best.score = bestScalable.score;
            strcpy( matchBuffer.data(), bestScalable.name );
            if ( qParseXFontName( matchBuffer, tokens ) ) {
                bestName.sprintf("-%s-%s-%s-%s-%s-%s-*-%i-75-75-%s-*-%s-%s",
                                 tokens[Foundry],
                                 tokens[Family],
                                 tokens[Weight_],
                                 tokens[Slant],
                                 tokens[Width],
                                 tokens[AddStyle],
                                 pointSize(),
                                 tokens[Spacing],
                                 tokens[CharsetRegistry],
                                 tokens[CharsetEncoding]);

                best.name = bestName.data();
                debug( "*** Scaled a font: [%s]", best.name );
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

void QFont::loadFont() const
{
    QString fontName;
    QString instanceID;
    QXFontData  *fd;


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
                                      family(),pointSize(),
                                      data->hintSetByUser ? styleHint : 0,
                                      charSet(), weight(), italic() ? 1 : 0,
                                      fixedPitch() ? 1 : 0 );

    debug( "instanceID = %s", instanceID.data() );

    fd = fontNameDict->find( instanceID );
    if (!fd) {
        if ( data->rawMode ) {
            if ( PRIV->findRawFont() )
                fontName = family();
            else
                fontName = PRIV->defaultFont();
        } else {
            fontName = PRIV->findFont(); // Returns a loadable font.
        }
	debug( "=== MATCHED FONT[%s] ", fontName.data() );

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
            if ( fd->font ) {			        // save for later
                debug( "min_byte1 = %i, max_byte1 = i", fd->font->min_byte1, 
                                                        fd->font->max_byte1 );
                debug( "min_cob2 = %i, max_cob2   = i", 
                       fd->font->min_char_or_byte2, 
                       fd->font->max_char_or_byte2 );
	        fontDict->insert( fontName, fd );
	    } else {
                warning( "QFont::loadFont: PANIC! Could not load X font!." );
                delete fd;
                return;
	    }
	    debug( "=== LOADED FONT" );
        }
        fontNameDict->insert( instanceID, fd );
    }
    data->xFontName = fd->name;

struct qat { char *nm; Atom a; };

qat fet[] = {
{ "XA_MIN_SPACE", XA_MIN_SPACE }, 
{ "XA_NORM_SPACE", XA_NORM_SPACE }, 
{ "XA_MAX_SPACE", XA_MAX_SPACE }, 
{ "XA_END_SPACE", XA_END_SPACE }, 
{ "XA_SUPERSCRIPT_X", XA_SUPERSCRIPT_X }, 
{ "XA_SUPERSCRIPT_Y", XA_SUPERSCRIPT_Y }, 
{ "XA_SUBSCRIPT_X", XA_SUBSCRIPT_X }, 
{ "XA_SUBSCRIPT_Y", XA_SUBSCRIPT_Y }, 
{ "XA_UNDERLINE_POSITION", XA_UNDERLINE_POSITION }, 
{ "XA_UNDERLINE_THICKNESS", XA_UNDERLINE_THICKNESS }, 
{ "XA_STRIKEOUT_ASCENT", XA_STRIKEOUT_ASCENT }, 
{ "XA_STRIKEOUT_DESCENT", XA_STRIKEOUT_DESCENT }, 
{ "XA_ITALIC_ANGLE", XA_ITALIC_ANGLE }, 
{ "XA_X_HEIGHT", XA_X_HEIGHT }, 
{ "XA_QUAD_WIDTH", XA_QUAD_WIDTH }, 
{ "XA_WEIGHT", XA_WEIGHT }, 
{ "XA_POINT_SIZE", XA_POINT_SIZE }, 
{ "XA_RESOLUTION", XA_RESOLUTION }, 
{ "xA_COPYRIGHT", XA_COPYRIGHT }, 
{ "xA_NOTICE", XA_NOTICE }, 
{ "xA_FONT_NAME", XA_FONT_NAME }, 
{ "xA_FAMILY_NAME", XA_FAMILY_NAME }, 
{ "xA_FULL_NAME", XA_FULL_NAME }, 
{ "XA_CAP_HEIGHT", XA_CAP_HEIGHT },
{ "", (Atom) 0 } };

//    qat *tmp = fet;

    XFontStruct *f = fd->font;

    XFontProp *tmp = f->properties;
    int cnt = f->n_properties;

    char *name, *str;
    ulong value;

    while( cnt-- ) {

        name = XGetAtomName( qXDisplay(), tmp->name );
        if ( tmp->card32 < 156 && tmp->card32 > 0 )
            str  = XGetAtomName( qXDisplay(), (Atom) tmp->card32 );
        else
            str = 0;

        if ( str )
            debug( "[%s] = %i \"%s\"", name, tmp->card32, str );
        else
            debug( "[%s] = %i", name, tmp->card32 );
        tmp++;
    }

/*
        if ( XGetFontProperty( f, tmp->a, &value ) ) {
            if ( tmp->nm[0] == 'X') {
            } else {
                debug( "[%s] = \"%s\"", tmp->nm,
                       XGetAtomName( qXDisplay(), (Atom) value ) );
            }
        } else {
                debug( "[%s] NOT DEFINED! " );
        }
        tmp++;
    }
*/
    data->f         = f;
    data->dirty     = FALSE;
    debug( "=== GOT FONT" );
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
    data->dirty         = TRUE;
}

QFontMetrics::QFontMetrics( const QFont &font )
{
    f    = &font;
    data = new QFontData;
    CHECK_PTR( data );
    data->dirty         = TRUE;
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

    char *tokens[qXFontFields];
    QString buffer( 255 );       // Used to hold parsed X font name


    buffer = f->data->xFontName.copy();
    if ( !qParseXFontName( buffer, tokens ) ) {
        debug("QFontMetrics::updateData: Internal error, reseting font data");
        debug("{%s}", f->data->xFontName.data() );       
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

    QString weightStr = tokens[Weight_];
    weightStr.lower();
    if ( weightStr == "medium" ) {
        data->weight = QFont::Normal;
    } else {
        if ( weightStr == "bold" ) {
            data->weight = QFont::Bold;
        } else {                             // ###
            data->weight = QFont::AnyWeight; // Not enough, must read font info
        }
    }

    if ( strcmp( tokens[ResolutionY], "75") != 0 ) { // If not 75 dpi
        data->pointSize = ( 2*data->pointSize*atoi(tokens[ResolutionY]) + 1 ) 
                          / ( 75 * 2 );  // Adjust actual pointsize
    }

}

