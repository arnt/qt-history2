#include "scriptenginearabic.h"

#include "private/qcomplextext_p.h"
#include "private/qunicodetables_p.h"
#include <stdlib.h>
#include "opentype.h"

// #### keep in sync with bits in opentype.cpp
enum Shape {
    XIsolated = 0x02,
    XFinal = 0x04,
    XInitial = 0x10,
    XMedial = 0x08
};

/*
  Two small helper functions for arabic shaping. They get the next shape causing character on either
  side of the char in question. Implements rule R1.

  leftChar() returns true if the char to the left is a left join-causing char
  rightChar() returns true if the char to the right is a right join-causing char
*/
static inline const QChar *prevChar( const QString &str, int pos )
{
    //qDebug("leftChar: pos=%d", pos);
    pos--;
    const QChar *ch = str.unicode() + pos;
    while( pos > -1 ) {
	if( !isMark( *ch ) )
	    return ch;
	pos--;
	ch--;
    }
    return &QChar::replacement;
}

static inline const QChar *nextChar( const QString &str, int pos)
{
    pos++;
    int len = str.length();
    const QChar *ch = str.unicode() + pos;
    while( pos < len ) {
	//qDebug("rightChar: %d isLetter=%d, joining=%d", pos, ch.isLetter(), ch.joining());
	if( !isMark( *ch ) )
	    return ch;
	// assume it's a transparent char, this might not be 100% correct
	pos++;
	ch++;
    }
    return &QChar::replacement;
}

/* and the same thing for logical ordering :)
 */
static inline bool prevLogicalCharJoins( const QString &str, int pos)
{
    return ( joining( *nextChar( str, pos ) ) != QChar::OtherJoining );
}

static inline bool nextLogicalCharJoins( const QString &str, int pos)
{
    QChar::Joining join = joining( *prevChar( str, pos ) );
    return ( join == QChar::Dual || join == QChar::Center );
}


static inline Shape glyphVariantLogical( const QString &str, int pos)
{
    // ignores L1 - L3, ligatures are job of the codec
    QChar::Joining joining = ::joining( str.unicode()[pos] );
    //qDebug("checking %x, joining=%d", str[pos].unicode(), joining);
    switch ( joining ) {
	case QChar::OtherJoining:
	case QChar::Center:
	    // these don't change shape
	    return XIsolated;
	case QChar::Right:
	    // only rule R2 applies
	    return ( nextLogicalCharJoins( str, pos ) ) ? XFinal : XIsolated;
	case QChar::Dual:
	    bool right = nextLogicalCharJoins( str, pos );
	    bool left = prevLogicalCharJoins( str, pos );
	    //qDebug("dual: right=%d, left=%d", right, left);
	    return ( right ) ? ( left ? XMedial : XFinal ) : ( left ? XInitial : XIsolated );
    }
    return XIsolated;
}


void ScriptEngineArabic::charAttributes( const QString &text, int from, int len, CharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	attributes[i].softBreak = FALSE;
	// ### remove nbsp?
	attributes[i].whiteSpace = uc[i].isSpace();
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = attributes[i].whiteSpace;
    }
}


void ScriptEngineArabic::shape( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( OpenTypeIface::Arabic ) ) {
	openTypeShape( openType, result );
	return;
    }

    ShapedItemPrivate *d = result->d;
    const QString &text = d->string;
    int from = d->from;
    int len = d->length;

    QString shaped = QComplexText::shapedString( text, from, len, QPainter::LTR );
    len = shaped.length();

    d->num_glyphs = len;
    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( shaped.unicode(), len, d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( shaped.unicode(), len, d->glyphs, &d->num_glyphs );
    }

    // ######## wrong due to ligatures!!!!!!
    heuristicSetGlyphAttributes( result );

    d->offsets = (Offset *) malloc( d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );
}


void ScriptEngineArabic::openTypeShape( const OpenTypeIface *openType, ShapedItem *result )
{
    ShapedItemPrivate *d = result->d;
    const QString &text = d->string;
    int from = d->from;
    int len = d->length;

    d->num_glyphs = len;
    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( text.unicode()+from, len, d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( text.unicode()+from, len, d->glyphs, &d->num_glyphs );
    }

    heuristicSetGlyphAttributes( result );

    d->offsets = (Offset *) malloc( d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );

    unsigned char fa[256];
    unsigned char *featuresToApply = fa;

    bool allocated = FALSE;
    if ( d->num_glyphs > 255 ) {
	featuresToApply = (unsigned char *)malloc( d->num_glyphs );
	allocated = TRUE;
    }

    for ( int i = 0; i < d->num_glyphs; i++ )
	featuresToApply[i] = glyphVariantLogical( text, from + i );

    ((OpenTypeIface *) openType)->applyGlyphSubstitutions( OpenTypeIface::Arabic, result, featuresToApply );

    if ( allocated )
	free( featuresToApply );
}
