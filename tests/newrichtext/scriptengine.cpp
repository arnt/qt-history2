#include "scriptengine.h"
#include <stdlib.h>

#include <qstring.h>
#include <qrect.h>
#include <private/qunicodetables_p.h>
#include "qtextengine.h"

#include <stdlib.h>
#include "opentype.h"
#include "qfont.h"


// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Basic processing
//
// --------------------------------------------------------------------------------------------------------------------------------------------

static inline void positionCluster( QScriptItem *item, int gfrom,  int glast )
{
    int nmarks = glast - gfrom;
    if ( nmarks <= 0 ) {
	qWarning( "positionCluster: no marks to position!" );
	return;
    }

    QShapedItem *shaped = item->shaped;
    QFontEngine *f = item->fontEngine;
    QGlyphMetrics baseInfo = f->boundingBox( shaped->glyphs[gfrom] );
    QRect baseRect( baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height );

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast );
//     qDebug( "baseInfo: %d/%d (%d/%d) off=%d/%d", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff );

    int size = f->ascent()/10;
    int offsetBase = (size - 4) / 4 + QMIN( size, 4 ) + 1;
//     qDebug("offset = %d", offsetBase );

    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;

    for( i = 1; i <= nmarks; i++ ) {
	glyph_t mark = shaped->glyphs[gfrom+i];
	QPoint p;
	QGlyphMetrics markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x, markInfo.y, markInfo.width, markInfo.height );

	int offset = offsetBase;
	unsigned char cmb = shaped->glyphAttributes[gfrom+i].combiningClass;

	// ### maybe the whole position determination should move down to heuristicSetGlyphAttributes. Would save some
	// bits  in the glyphAttributes structure.
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew, lao and thai.

	    // for Lao and Thai marks with class 0, see below ( heuristicSetGlyphAttributes )

	    // add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 && offset < 3 )
		offset +=1;
 	    // below
	    if ( (cmb >= 10 && cmb <= 18) ||
		 cmb == 20 || cmb == 22 ||
		 cmb == 29 || cmb == 32 )
		cmb = QChar::Combining_Below;
	    // above
	    else if ( cmb == 23 || cmb == 27 || cmb == 28 ||
		      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36 ) )
		cmb = QChar::Combining_Above;
	    //below-right
	    else if ( cmb == 9 || cmb == 103 || cmb == 118 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 || cmb == 122 )
		cmb = QChar::Combining_AboveRight;
	    else if ( cmb == 25 )
		cmb = QChar::Combining_AboveLeft;
	    // fixed:
	    //  19 21

	}

	// combining marks of different class don't interact. Reset the rectangle.
	if ( cmb != lastCmb ) {
	    //qDebug( "resetting rect" );
	    attachmentRect = baseRect;
	}

	switch( cmb ) {
	case QChar::Combining_DoubleBelow:
		// ### wrong in rtl context!
	case QChar::Combining_BelowLeft:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowLeftAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    break;
	case QChar::Combining_Below:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_BelowRight:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowRightAttached:
	    p += attachmentRect.bottomRight() - markRect.topRight();
	    break;
	    case QChar::Combining_Left:
	    p += QPoint( -offset, 0 );
	case QChar::Combining_LeftAttached:
	    break;
	    case QChar::Combining_Right:
	    p += QPoint( offset, 0 );
	case QChar::Combining_RightAttached:
	    break;
	case QChar::Combining_DoubleAbove:
	    // ### wrong in RTL context!
	case QChar::Combining_AboveLeft:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveLeftAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    break;
	    case QChar::Combining_Above:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_AboveRight:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveRightAttached:
	    p += attachmentRect.topRight() - markRect.bottomRight();
	    break;

	case QChar::Combining_IotaSubscript:
	    default:
		break;
	}
// 	qDebug( "char=%x combiningClass = %d offset=%d/%d", mark, cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	attachmentRect |= markRect;
	lastCmb = cmb;
	shaped->offsets[gfrom+i].x = p.x() - baseInfo.xoff;
	shaped->offsets[gfrom+i].y = p.y() - baseInfo.yoff;
	shaped->advances[gfrom+i].x = 0;
	shaped->advances[gfrom+i].y = 0;
    }
}


static void heuristicPosition( QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;;

    int cEnd = -1;
    int i = shaped->num_glyphs;
    while ( i-- ) {
	if ( cEnd == -1 && shaped->glyphAttributes[i].mark ) {
	    cEnd = i;
	} else if ( cEnd != -1 && !shaped->glyphAttributes[i].mark ) {
	    positionCluster( item, i, cEnd );
	    cEnd = -1;
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
// and no reordering.
// also computes logClusters heuristically
static void heuristicSetGlyphAttributes( const QString &string, int from, int len, QScriptItem *item )
{
    // ### zeroWidth and justification are missing here!!!!!
    QShapedItem *shaped = item->shaped;

    if ( shaped->num_glyphs != len )
	qWarning("QScriptEngine::heuristicSetGlyphAttributes: char length and num glyphs disagree" );

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", shaped->num_glyphs);

    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, shaped->num_glyphs * sizeof( GlyphAttributes ) );

    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, shaped->num_glyphs * sizeof( unsigned short ) );

    // honour the logClusters array if it exists.
    const QChar *uc = string.unicode() + from;

    for ( int i = 0; i < shaped->num_glyphs; i++ )
	shaped->logClusters[i] = i;

    int pos = 1;

    // first char in a run is never (treated as) a mark
    int cStart = 0;
    shaped->glyphAttributes[0].mark = FALSE;
    shaped->glyphAttributes[0].clusterStart = TRUE;

    while ( pos < len ) {
	if ( ::category( uc[pos] ) == QChar::Mark_NonSpacing ) {
	    shaped->glyphAttributes[pos].mark = TRUE;
	    shaped->glyphAttributes[pos].clusterStart = FALSE;
	    int cmb = combiningClass( uc[pos] );

	    if ( cmb == 0 ) {
		// Fix 0 combining classes
		if ( uc[pos].row() == 0x0e ) {
		    // thai or lao
		    unsigned char col = uc[pos].cell();
		    if ( col == 0x31 ||
			 col == 0x34 ||
			 col == 0x35 ||
			 col == 0x36 ||
			 col == 0x37 ||
			 col == 0x47 ||
			 col == 0x4c ||
			 col == 0x4d ||
			 col == 0x4e ) {
			cmb = QChar::Combining_AboveRight;
		    } else if ( col == 0xb1 ||
				col == 0xb4 ||
				col == 0xb5 ||
				col == 0xb6 ||
				col == 0xb7 ||
				col == 0xbb ||
				col == 0xcc ||
				col == 0xcd ) {
			cmb = QChar::Combining_Above;
		    } else if ( col == 0xbc ) {
			cmb = QChar::Combining_Below;
		    }
		}
	    }

	    shaped->glyphAttributes[pos].combiningClass = cmb;
	    // 		qDebug("found a mark at position %d", pos );
	    shaped->logClusters[pos] = cStart;
	} else {
	    shaped->glyphAttributes[pos].mark = FALSE;
	    shaped->glyphAttributes[pos].clusterStart = TRUE;
	    shaped->glyphAttributes[pos].combiningClass = 0;
	    cStart = pos;
	}
	pos++;
    }
}


void q_calculateAdvances( QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;
    shaped->offsets = (offset_t *) realloc( shaped->offsets, shaped->num_glyphs * sizeof( offset_t ) );
    memset( shaped->offsets, 0, shaped->num_glyphs * sizeof( offset_t ) );
    shaped->advances = (offset_t *) realloc( shaped->advances, shaped->num_glyphs * sizeof( offset_t ) );
    item->ascent = item->fontEngine->ascent();
    item->descent = item->fontEngine->descent();
    item->width = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	QGlyphMetrics gi = item->fontEngine->boundingBox( shaped->glyphs[i] );
	shaped->advances[i].x = gi.xoff;
	shaped->advances[i].y = gi.yoff;
// 	qDebug("setting advance of glyph %d to %d", i, gi.xoff );
	int y = shaped->offsets[i].y + gi.y;
	item->ascent = QMAX( item->ascent, -y );
	item->descent = QMAX( item->descent, y + gi.height );
	item->width += gi.xoff;
    }
}

static void basic_shape( int /*script*/, const QString &string, int from, int len, QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;
    shaped->num_glyphs = len;
    shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
    int error = item->fontEngine->stringToCMap( string.unicode() + from, len, shaped->glyphs, &shaped->num_glyphs );
    if ( error == QFontEngine::OutOfMemory ) {
	shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
	item->fontEngine->stringToCMap( string.unicode() + from, len, shaped->glyphs, &shaped->num_glyphs );
    }

    heuristicSetGlyphAttributes( string, from, len, item );
    q_calculateAdvances( item );
    heuristicPosition( item );
}

static void basic_attributes( const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	// ### remove nbsp?
	attributes[i].whiteSpace = ::isSpace( uc[i] );
	attributes[i].softBreak = attributes[i].whiteSpace;
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = attributes[i].whiteSpace;
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Middle eastern languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

/*
   Arabic shaping obeys a number of rules according to the joining classes (see Unicode book, section on
   arabic).

   Each unicode char has a joining class (right, dual (left&right), center (joincausing) or transparent).
   transparent joining is not encoded in QChar::joining(), but applies to all combining marks and format marks.

   Right join-causing: dual + center
   Left join-causing: dual + right + center

   Rules are as follows (for a string already in visual order, as we have it here):

   R1 Transparent characters do not affect joining behaviour.
   R2 A right joining character, that has a right join-causing char on the right will get form XRight
   (R3 A left joining character, that has a left join-causing char on the left will get form XLeft)
   Note: the above rule is meaningless, as there are no pure left joining characters defined in Unicode
   R4 A dual joining character, that has a left join-causing char on the left and a right join-causing char on
	     the right will get form XMedial
   R5  A dual joining character, that has a right join causing char on the right, and no left join causing char on the left
	 will get form XRight
   R6 A dual joining character, that has a  left join causing char on the left, and no right join causing char on the right
	 will get form XLeft
   R7 Otherwise the character will get form XIsolated

   Additionally we have to do the minimal ligature support for lam-alef ligatures:

   L1 Transparent characters do not affect ligature behaviour.
   L2 Any sequence of Alef(XRight) + Lam(XMedial) will form the ligature Alef.Lam(XLeft)
   L3 Any sequence of Alef(XRight) + Lam(XLeft) will form the ligature Alef.Lam(XIsolated)

   The two functions defined in this class do shaping in visual and logical order. For logical order just replace right with
   previous and left with next in the above rules ;-)
*/

/*
  Two small helper functions for arabic shaping. They get the next shape causing character on either
  side of the char in question. Implements rule R1.

  leftChar() returns true if the char to the left is a left join-causing char
  rightChar() returns true if the char to the right is a right join-causing char
*/


enum Shape {
    XIsolated,
    XFinal,
    XInitial,
    XMedial
};

// ### keep in sync with table entries in opentype.cpp
static const unsigned short shapeToOpenTypeBit[] = {
    0x01,
    0x02,
    0x08,
    0x04
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
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
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
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
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


// The unicode to unicode shaping codec.
// does only presentation forms B at the moment, but that should be enough for
// simple display
static const ushort arabicUnicodeMapping[256][2] = {
    // base of shaped forms, and number-1 of them ( 0 for non shaping,
    // 1 for right binding and 3 for dual binding

    // These are just the glyphs available in Unicode,
    // some characters are in R class, but have no glyphs in Unicode.

    { 0x0600, 0 }, // 0x0600
    { 0x0601, 0 }, // 0x0601
    { 0x0602, 0 }, // 0x0602
    { 0x0603, 0 }, // 0x0603
    { 0x0604, 0 }, // 0x0604
    { 0x0605, 0 }, // 0x0605
    { 0x0606, 0 }, // 0x0606
    { 0x0607, 0 }, // 0x0607
    { 0x0608, 0 }, // 0x0608
    { 0x0609, 0 }, // 0x0609
    { 0x060A, 0 }, // 0x060A
    { 0x060B, 0 }, // 0x060B
    { 0x060C, 0 }, // 0x060C
    { 0x060D, 0 }, // 0x060D
    { 0x060E, 0 }, // 0x060E
    { 0x060F, 0 }, // 0x060F

    { 0x0610, 0 }, // 0x0610
    { 0x0611, 0 }, // 0x0611
    { 0x0612, 0 }, // 0x0612
    { 0x0613, 0 }, // 0x0613
    { 0x0614, 0 }, // 0x0614
    { 0x0615, 0 }, // 0x0615
    { 0x0616, 0 }, // 0x0616
    { 0x0617, 0 }, // 0x0617
    { 0x0618, 0 }, // 0x0618
    { 0x0619, 0 }, // 0x0619
    { 0x061A, 0 }, // 0x061A
    { 0x061B, 0 }, // 0x061B
    { 0x061C, 0 }, // 0x061C
    { 0x061D, 0 }, // 0x061D
    { 0x061E, 0 }, // 0x061E
    { 0x061F, 0 }, // 0x061F

    { 0x0620, 0 }, // 0x0620
    { 0xFE80, 0 }, // 0x0621            HAMZA
    { 0xFE81, 1 }, // 0x0622    R       ALEF WITH MADDA ABOVE
    { 0xFE83, 1 }, // 0x0623    R       ALEF WITH HAMZA ABOVE
    { 0xFE85, 1 }, // 0x0624    R       WAW WITH HAMZA ABOVE
    { 0xFE87, 1 }, // 0x0625    R       ALEF WITH HAMZA BELOW
    { 0xFE89, 3 }, // 0x0626    D       YEH WITH HAMZA ABOVE
    { 0xFE8D, 1 }, // 0x0627    R       ALEF
    { 0xFE8F, 3 }, // 0x0628    D       BEH
    { 0xFE93, 1 }, // 0x0629    R       TEH MARBUTA
    { 0xFE95, 3 }, // 0x062A    D       TEH
    { 0xFE99, 3 }, // 0x062B    D       THEH
    { 0xFE9D, 3 }, // 0x062C    D       JEEM
    { 0xFEA1, 3 }, // 0x062D    D       HAH
    { 0xFEA5, 3 }, // 0x062E    D       KHAH
    { 0xFEA9, 1 }, // 0x062F    R       DAL

    { 0xFEAB, 1 }, // 0x0630    R       THAL
    { 0xFEAD, 1 }, // 0x0631    R       REH
    { 0xFEAF, 1 }, // 0x0632    R       ZAIN
    { 0xFEB1, 3 }, // 0x0633    D       SEEN
    { 0xFEB5, 3 }, // 0x0634    D       SHEEN
    { 0xFEB9, 3 }, // 0x0635    D       SAD
    { 0xFEBD, 3 }, // 0x0636    D       DAD
    { 0xFEC1, 3 }, // 0x0637    D       TAH
    { 0xFEC5, 3 }, // 0x0638    D       ZAH
    { 0xFEC9, 3 }, // 0x0639    D       AIN
    { 0xFECD, 3 }, // 0x063A    D       GHAIN
    { 0x063B, 0 }, // 0x063B
    { 0x063C, 0 }, // 0x063C
    { 0x063D, 0 }, // 0x063D
    { 0x063E, 0 }, // 0x063E
    { 0x063F, 0 }, // 0x063F

    { 0x0640, 0 }, // 0x0640    C       TATWEEL // ### Join Causing, only one glyph
    { 0xFED1, 3 }, // 0x0641    D       FEH
    { 0xFED5, 3 }, // 0x0642    D       QAF
    { 0xFED9, 3 }, // 0x0643    D       KAF
    { 0xFEDD, 3 }, // 0x0644    D       LAM
    { 0xFEE1, 3 }, // 0x0645    D       MEEM
    { 0xFEE5, 3 }, // 0x0646    D       NOON
    { 0xFEE9, 3 }, // 0x0647    D       HEH
    { 0xFEED, 1 }, // 0x0648    R       WAW
    { 0x0649, 0 }, // 0x0649            ALEF MAKSURA // ### Dual, glyphs not consecutive, handle in code.
    { 0xFEF1, 3 }, // 0x064A    D       YEH
    { 0x064B, 0 }, // 0x064B
    { 0x064C, 0 }, // 0x064C
    { 0x064D, 0 }, // 0x064D
    { 0x064E, 0 }, // 0x064E
    { 0x064F, 0 }, // 0x064F

    { 0x0650, 0 }, // 0x0650
    { 0x0651, 0 }, // 0x0651
    { 0x0652, 0 }, // 0x0652
    { 0x0653, 0 }, // 0x0653
    { 0x0654, 0 }, // 0x0654
    { 0x0655, 0 }, // 0x0655
    { 0x0656, 0 }, // 0x0656
    { 0x0657, 0 }, // 0x0657
    { 0x0658, 0 }, // 0x0658
    { 0x0659, 0 }, // 0x0659
    { 0x065A, 0 }, // 0x065A
    { 0x065B, 0 }, // 0x065B
    { 0x065C, 0 }, // 0x065C
    { 0x065D, 0 }, // 0x065D
    { 0x065E, 0 }, // 0x065E
    { 0x065F, 0 }, // 0x065F

    { 0x0660, 0 }, // 0x0660
    { 0x0661, 0 }, // 0x0661
    { 0x0662, 0 }, // 0x0662
    { 0x0663, 0 }, // 0x0663
    { 0x0664, 0 }, // 0x0664
    { 0x0665, 0 }, // 0x0665
    { 0x0666, 0 }, // 0x0666
    { 0x0667, 0 }, // 0x0667
    { 0x0668, 0 }, // 0x0668
    { 0x0669, 0 }, // 0x0669
    { 0x066A, 0 }, // 0x066A
    { 0x066B, 0 }, // 0x066B
    { 0x066C, 0 }, // 0x066C
    { 0x066D, 0 }, // 0x066D
    { 0x066E, 0 }, // 0x066E
    { 0x066F, 0 }, // 0x066F

    { 0x0670, 0 }, // 0x0670
    { 0xFB50, 1 }, // 0x0671    R       ALEF WASLA
    { 0x0672, 0 }, // 0x0672
    { 0x0673, 0 }, // 0x0673
    { 0x0674, 0 }, // 0x0674
    { 0x0675, 0 }, // 0x0675
    { 0x0676, 0 }, // 0x0676
    { 0x0677, 0 }, // 0x0677
    { 0x0678, 0 }, // 0x0678
    { 0xFB66, 3 }, // 0x0679    D       TTEH
    { 0xFB5E, 3 }, // 0x067A    D       TTEHEH
    { 0xFB52, 3 }, // 0x067B    D       BEEH
    { 0x067C, 0 }, // 0x067C
    { 0x067D, 0 }, // 0x067D
    { 0xFB56, 3 }, // 0x067E    D       PEH
    { 0xFB62, 3 }, // 0x067F    D       TEHEH

    { 0xFB5A, 3 }, // 0x0680    D       BEHEH
    { 0x0681, 0 }, // 0x0681
    { 0x0682, 0 }, // 0x0682
    { 0xFB76, 3 }, // 0x0683    D       NYEH
    { 0xFB72, 3 }, // 0x0684    D       DYEH
    { 0x0685, 0 }, // 0x0685
    { 0xFB7A, 3 }, // 0x0686    D       TCHEH
    { 0xFB7E, 3 }, // 0x0687    D       TCHEHEH
    { 0xFB88, 1 }, // 0x0688    R       DDAL
    { 0x0689, 0 }, // 0x0689
    { 0x068A, 0 }, // 0x068A
    { 0x068B, 0 }, // 0x068B
    { 0xFB84, 1 }, // 0x068C    R       DAHAL
    { 0xFB82, 1 }, // 0x068D    R       DDAHAL
    { 0xFB86, 1 }, // 0x068E    R       DUL
    { 0x068F, 0 }, // 0x068F

    { 0x0690, 0 }, // 0x0690
    { 0xFB8C, 1 }, // 0x0691    R       RREH
    { 0x0692, 0 }, // 0x0692
    { 0x0693, 0 }, // 0x0693
    { 0x0694, 0 }, // 0x0694
    { 0x0695, 0 }, // 0x0695
    { 0x0696, 0 }, // 0x0696
    { 0x0697, 0 }, // 0x0697
    { 0xFB8A, 1 }, // 0x0698    R       JEH
    { 0x0699, 0 }, // 0x0699
    { 0x069A, 0 }, // 0x069A
    { 0x069B, 0 }, // 0x069B
    { 0x069C, 0 }, // 0x069C
    { 0x069D, 0 }, // 0x069D
    { 0x069E, 0 }, // 0x069E
    { 0x069F, 0 }, // 0x069F

    { 0x06A0, 0 }, // 0x06A0
    { 0x06A1, 0 }, // 0x06A1
    { 0x06A2, 0 }, // 0x06A2
    { 0x06A3, 0 }, // 0x06A3
    { 0xFB6A, 3 }, // 0x06A4    D       VEH
    { 0x06A5, 0 }, // 0x06A5
    { 0xFB6E, 3 }, // 0x06A6    D       PEHEH
    { 0x06A7, 0 }, // 0x06A7
    { 0x06A8, 0 }, // 0x06A8
    { 0xFB8E, 3 }, // 0x06A9    D       KEHEH
    { 0x06AA, 0 }, // 0x06AA
    { 0x06AB, 0 }, // 0x06AB
    { 0x06AC, 0 }, // 0x06AC
    { 0xFBD3, 3 }, // 0x06AD    D       NG
    { 0x06AE, 0 }, // 0x06AE
    { 0xFB92, 3 }, // 0x06AF    D       GAF

    { 0x06B0, 0 }, // 0x06B0
    { 0xFB9A, 3 }, // 0x06B1    D       NGOEH
    { 0x06B2, 0 }, // 0x06B2
    { 0xFB96, 3 }, // 0x06B3    D       GUEH
    { 0x06B4, 0 }, // 0x06B4
    { 0x06B5, 0 }, // 0x06B5
    { 0x06B6, 0 }, // 0x06B6
    { 0x06B7, 0 }, // 0x06B7
    { 0x06B8, 0 }, // 0x06B8
    { 0x06B9, 0 }, // 0x06B9
    { 0x06BA, 0 }, // 0x06BA
    { 0xFBA0, 3 }, // 0x06BB    D       RNOON
    { 0x06BC, 0 }, // 0x06BC
    { 0x06BD, 0 }, // 0x06BD
    { 0xFBAA, 3 }, // 0x06BE    D       HEH DOACHASHMEE
    { 0x06BF, 0 }, // 0x06BF

    { 0xFBA4, 1 }, // 0x06C0    R       HEH WITH YEH ABOVE
    { 0xFBA6, 3 }, // 0x06C1    D       HEH GOAL
    { 0x06C2, 0 }, // 0x06C2
    { 0x06C3, 0 }, // 0x06C3
    { 0x06C4, 0 }, // 0x06C4
    { 0xFBE0, 1 }, // 0x06C5    R       KIRGHIZ OE
    { 0xFBD9, 1 }, // 0x06C6    R       OE
    { 0xFBD7, 1 }, // 0x06C7    R       U
    { 0xFBDB, 1 }, // 0x06C8    R       YU
    { 0xFBE2, 1 }, // 0x06C9    R       KIRGHIZ YU
    { 0x06CA, 0 }, // 0x06CA
    { 0xFBDE, 1 }, // 0x06CB    R       VE
    { 0xFBFC, 3 }, // 0x06CC    D       FARSI YEH
    { 0x06CD, 0 }, // 0x06CD
    { 0x06CE, 0 }, // 0x06CE
    { 0x06CF, 0 }, // 0x06CF

    { 0xFBE4, 3 }, // 0x06D0    D       E
    { 0x06D1, 0 }, // 0x06D1
    { 0xFBAE, 1 }, // 0x06D2    R       YEH BARREE
    { 0xFBB0, 1 }, // 0x06D3    R       YEH BARREE WITH HAMZA ABOVE
    { 0x06D4, 0 }, // 0x06D4
    { 0x06D5, 0 }, // 0x06D5
    { 0x06D6, 0 }, // 0x06D6
    { 0x06D7, 0 }, // 0x06D7
    { 0x06D8, 0 }, // 0x06D8
    { 0x06D9, 0 }, // 0x06D9
    { 0x06DA, 0 }, // 0x06DA
    { 0x06DB, 0 }, // 0x06DB
    { 0x06DC, 0 }, // 0x06DC
    { 0x06DD, 0 }, // 0x06DD
    { 0x06DE, 0 }, // 0x06DE
    { 0x06DF, 0 }, // 0x06DF

    { 0x06E0, 0 }, // 0x06E0
    { 0x06E1, 0 }, // 0x06E1
    { 0x06E2, 0 }, // 0x06E2
    { 0x06E3, 0 }, // 0x06E3
    { 0x06E4, 0 }, // 0x06E4
    { 0x06E5, 0 }, // 0x06E5
    { 0x06E6, 0 }, // 0x06E6
    { 0x06E7, 0 }, // 0x06E7
    { 0x06E8, 0 }, // 0x06E8
    { 0x06E9, 0 }, // 0x06E9
    { 0x06EA, 0 }, // 0x06EA
    { 0x06EB, 0 }, // 0x06EB
    { 0x06EC, 0 }, // 0x06EC
    { 0x06ED, 0 }, // 0x06ED
    { 0x06EE, 0 }, // 0x06EE
    { 0x06EF, 0 }, // 0x06EF

    { 0x06F0, 0 }, // 0x06F0
    { 0x06F1, 0 }, // 0x06F1
    { 0x06F2, 0 }, // 0x06F2
    { 0x06F3, 0 }, // 0x06F3
    { 0x06F4, 0 }, // 0x06F4
    { 0x06F5, 0 }, // 0x06F5
    { 0x06F6, 0 }, // 0x06F6
    { 0x06F7, 0 }, // 0x06F7
    { 0x06F8, 0 }, // 0x06F8
    { 0x06F9, 0 }, // 0x06F9
    { 0x06FA, 0 }, // 0x06FA
    { 0x06FB, 0 }, // 0x06FB
    { 0x06FC, 0 }, // 0x06FC
    { 0x06FD, 0 }, // 0x06FD
    { 0x06FE, 0 }, // 0x06FE
    { 0x06FF, 0 }  // 0x06FF
};

// the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, this table does
static const ushort alefMaksura[4] = {0xFEEF, 0xFEF0, 0xFBE8, 0xFBE9};

// this is a bit tricky. Alef always binds to the right, so the second parameter descibing the shape
// of the lam can be either initial of medial. So initial maps to the isolated form of the ligature,
// medial to the final form
static const ushort arabicUnicodeLamAlefMapping[6][4] = {
    { 0xfffd, 0xfffd, 0xfef5, 0xfef6 }, // 0x622        R       Alef with Madda above
    { 0xfffd, 0xfffd, 0xfef7, 0xfef8 }, // 0x623        R       Alef with Hamza above
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x624        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfef9, 0xfefa }, // 0x625        R       Alef with Hamza below
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x626        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfefb, 0xfefc }  // 0x627        R       Alef
};

static inline int getShape( uchar cell, int shape )
{
    // the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, handle this here
    uint ch = ( cell != 0x49 ) ? arabicUnicodeMapping[cell][0] + shape
	    		       : alefMaksura[shape] ;
    return ch;
}


static void shapedString(const QString& uc, int from, int len, QChar *shapeBuffer, int *shapedLength,
			 bool reverse, GlyphAttributes *attrs, unsigned short *logClusters )
{
    if( len < 0 ) {
	len = uc.length() - from;
    } else if( len == 0 ) {
	*shapedLength = 0;
	return;
    }

    const QChar *ch = uc.unicode() + from;
    QChar *data = shapeBuffer;
    int clusterStart = 0;

    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	bool zeroWidth = FALSE;
	int gpos = data - shapeBuffer;

	if ( r != 0x06 ) {
	    if ( r == 0x20 ) {
		uchar c = ch->cell();
		if (c == 0x0c || c == 0x0d)
		    // remove ZWJ and ZWNJ
		    zeroWidth = TRUE;
	    }
	    if ( reverse )
		*data = mirroredChar( *ch );
	    else
		*data = *ch;
	} else {
	    uchar c = ch->cell();
	    int pos = i + from;
	    int shape = glyphVariantLogical( uc, pos );
// 	    qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, getShape(c, shape));
	    // take care of lam-alef ligatures (lam right of alef)
	    ushort map;
	    switch ( c ) {
		case 0x44: { // lam
		    const QChar *pch = nextChar( uc, pos );
		    if ( pch->row() == 0x06 ) {
			switch ( pch->cell() ) {
			    case 0x22:
			    case 0x23:
			    case 0x25:
			    case 0x27:
// 				qDebug(" lam of lam-alef ligature");
				map = arabicUnicodeLamAlefMapping[pch->cell() - 0x22][shape];
				goto next;
			    default:
				break;
			}
		    }
		    break;
		}
		case 0x22: // alef with madda
		case 0x23: // alef with hamza above
		case 0x25: // alef with hamza below
		case 0x27: // alef
		    if ( prevChar( uc, pos )->unicode() == 0x0644 ) {
			// have a lam alef ligature
			//qDebug(" alef of lam-alef ligature");
			goto skip;
		    }
		default:
		    break;
	    }
	    map = getShape( c, shape );
	next:
	    *data = map;
	}
	attrs[gpos].zeroWidth = zeroWidth;
	if ( ::category( *ch ) == QChar::Mark_NonSpacing ) {
	    attrs[gpos].mark = TRUE;
// 	    qDebug("glyph %d (char %d) is mark!", gpos, i );
	} else {
	    attrs[gpos].mark = FALSE;
	    clusterStart = data - shapeBuffer;
	}
	attrs[gpos].clusterStart = !attrs[gpos].mark;
	attrs[gpos].combiningClass = combiningClass( *ch );
	data++;
    skip:
	ch++;
	logClusters[i] = clusterStart;
    }
    *shapedLength = data - shapeBuffer;
}

static void openTypeShape( int script, const QOpenType *openType, const QString &string, int from,
			   int len, QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;

    shaped->num_glyphs = len;
    shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
    int error = item->fontEngine->stringToCMap( string.unicode()+from, len, shaped->glyphs, &shaped->num_glyphs );
    if ( error == QFontEngine::OutOfMemory ) {
	shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
	item->fontEngine->stringToCMap( string.unicode()+from, len, shaped->glyphs, &shaped->num_glyphs );
    }

    heuristicSetGlyphAttributes( string, from, len, item );

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;

    bool allocated = FALSE;
    if ( shaped->num_glyphs > 255 ) {
	featuresToApply = (unsigned short *)malloc( shaped->num_glyphs );
	allocated = TRUE;
    }

    for ( int i = 0; i < shaped->num_glyphs; i++ )
	featuresToApply[i] = shapeToOpenTypeBit[glyphVariantLogical( string, from + i )];

    ((QOpenType *) openType)->apply( script, featuresToApply, item, len );

    if ( allocated )
	free( featuresToApply );
}


static void arabic_attributes( const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	// ### remove nbsp?
	attributes[i].whiteSpace = ::isSpace( uc[i] );
	attributes[i].softBreak = attributes[i].whiteSpace;
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = attributes[i].whiteSpace;
    }
}


static void arabic_shape( int /*script*/, const QString &string, int from, int len, QScriptItem *si )
{
    QOpenType *openType = si->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Arabic ) ) {
	openTypeShape( QFont::Arabic, openType, string,  from,  len, si );
	return;
    }

    const QString &text = string;
    QShapedItem *shaped = si->shaped;

    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, len * sizeof( GlyphAttributes ) );
    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, len * sizeof( unsigned short ) );
    QChar *shapedChars = (QChar *)malloc( len * sizeof( QChar ) );

    shapedString( text, from, len, shapedChars, &shaped->num_glyphs, (si->analysis.bidiLevel%2),
		  shaped->glyphAttributes, shaped->logClusters );

    shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
    int error = si->fontEngine->stringToCMap( shapedChars, shaped->num_glyphs, shaped->glyphs, &shaped->num_glyphs );
    if ( error == QFontEngine::OutOfMemory ) {
	shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
	si->fontEngine->stringToCMap( shapedChars, shaped->num_glyphs, shaped->glyphs, &shaped->num_glyphs );
    }

    heuristicSetGlyphAttributes( string, from, len, si );
    heuristicPosition( si );
}


static void syriac_shape( int script, const QString &string, int from, int len, QScriptItem *item )
{
    QOpenType *openType = item->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Syriac ) ) {
	openTypeShape( QFont::Syriac, openType, string, from, len, item );
	return;
    }
    basic_shape( script, string, from, len, item );
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Indic languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Other,
};

static unsigned char indicForms[0xe00-0x900] = {
    // Devangari
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, StressMark, StressMark, StressMark,
    StressMark, Unknown, Unknown, Unknown,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Other, Other, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Bengali
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, VowelMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    Other, Other, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gurmukhi
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    // Gujarati
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    // Oriya
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    //Tamil
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Telugu
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    // Kannada
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    // Malayalam
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    // Sinhala
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid
};

static inline Form form( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < 0x900 || uc > 0xdff ) {
	if ( uc == 0x25cc )
	    return Consonant;
	return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline bool isRa( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < 0x900 || uc > 0xd80 )
	return false;
    return ((uc & 0x7f) == 0x30 );
}

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split
};

static unsigned char indicPosition[0xe00-0x900] = {
    // Devanagari
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Bengali
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, Below,
    Below, None, None, Pre,
    Pre, None, None, Split,
    Split, Below, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gurmukhi
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gujarati
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Oriya
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Tamil
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Telugu
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Kannada
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Malayalam
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Sinhala
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
};

static inline Position position( const QChar &ch ) {
    unsigned short uc = ch.unicode();
    if ( uc < 0x900 && uc > 0xdff )
	return None;
    return (Position) indicPosition[uc-0x900];
}


/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   // ### check the above is correct

   We return syllable boundaries on invalid combinations aswell
*/
static int nextSyllableBoundary( int script, const QString &s, int start, int end, bool *invalid )
{
    *invalid = FALSE;
//     qDebug("nextSyllableBoundary: start=%d, end=%d", start, end );
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form( uc[pos] );
//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode() );
    pos++;

    if ( state != Consonant && state != IndependentVowel ) {
	if ( state != Other )
	    *invalid = TRUE;
	goto finish;
    }

    while ( pos < end - start ) {
	Form newState = form( uc[pos] );
// 	qDebug("state[%d]=%d (uc=%4x)", pos, newState, uc[pos].unicode() );
	switch( newState ) {
	case Consonant:
	    if ( state == Halant )
		break;
	    goto finish;
	case Halant:
	    if ( state == Nukta || state == Consonant )
		break;
	    goto finish;
	case Nukta:
	    if ( state == Consonant )
		break;
	    goto finish;
	case StressMark:
	    if ( state == VowelMark )
		break;
	    // fall through
	case VowelMark:
	    if ( state == Matra || state == IndependentVowel )
		break;
	    // fall through
	case Matra:
	    if ( state == Consonant || state == Nukta )
		break;
	    // ### not sure if this is correct. If it is, does it apply only to Bengali or should
	    // it work for all Indic languages?
	    // the combination Independent_A + Vowel Sign AA is allowed.
	    if ( script == QFont::Bengali && uc[pos].unicode() == 0x9be && uc[pos-1].unicode() == 0x985 )
		break;
	    if ( script == QFont::Tamil && state == Matra ) {
		if ( uc[pos-1].unicode() == 0x0bc6 &&
		     ( uc[pos].unicode() == 0xbbe || uc[pos].unicode() == 0xbd7 ) )
		    break;
		if ( uc[pos-1].unicode() == 0x0bc7 && uc[pos].unicode() == 0xbbe )
		    break;
	    }
	    goto finish;

	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    goto finish;
	}
	state = newState;
	pos++;
    }
 finish:
    return pos+start;
}


static QString devanagariReorder( const QString &string, int start, int end, unsigned short *featuresToApply,
				  GlyphAttributes *attributes, bool invalid )
{
    int len = end - start;

    QString reordered = string.mid( start, len );
    if ( invalid ) {
	reordered = QChar( 0x25cc ) + reordered;
	len++;
    }

    // in case mid() returns the whole string!
    reordered.setLength( reordered.length() );

    QChar *uc = (QChar *)reordered.unicode();

    for ( int i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[0].clusterStart = FALSE;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {
	bool reph = FALSE;
	if ( len > 2 && isRa( uc[0] ) && form( uc[1] ) == Halant ) {
	    reph = TRUE;
// 	    qDebug("Reph");
	}

	// Rule 1: find base consonant
	int lastConsonant = 0;
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		if ( !lastConsonant )
		    lastConsonant = i;
		// ### The MS specs says, that this should be done only if the syllable starts with a reph,
		// but they seem to act differently.
		if ( /*!reph ||*/ !isRa( uc[i] ) ) {
		    base = i;
		    break;
		}
	    }
	}
	if ( reph && base == 0 )
	    base = lastConsonant;

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

	// Rule 2: move halant of base consonant to last one. Only
	// possible if we skipped consonants while finding the base
	if ( lastConsonant != base && form( uc[base+1] ) == Halant ) {
// 	    qDebug("moving halant from %d to %d!", base+1, lastConsonant);
	    QChar halant = uc[base+1];
	    for ( int i = base+1; i < lastConsonant; i++ )
		uc[i] = uc[i+1];
	    uc[lastConsonant] = halant;

	}

	// Rule 3: Move reph to follow post base matra
	if ( reph ) {
	    int toPos = base+1;
	    if ( toPos < len && form( uc[toPos] ) == Nukta )
		toPos++;
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
// 	    qDebug("moving reph from %d to %d", 0, toPos );
	    QChar ra = uc[0];
	    QChar halant = uc[1];
	    for ( int i = 2; i < toPos; i++ )
		uc[i-2] = uc[i];
	    uc[toPos-2] = ra;
	    uc[toPos-1] = halant;
	    featuresToApply[toPos-2] |= RephFeature;
	    featuresToApply[toPos-1] |= RephFeature;
	    base -= 2;
	}
    }

    // Rule 4: split two part matras into parts
    // doesn't apply for devanagari

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base+1;
    if ( fixed < len && form( uc[fixed] ) == Nukta )
	fixed++;
    if ( fixed < len - 1 && isRa( uc[fixed] ) && form( uc[fixed+1] ) == Halant )
	fixed += 2;


    // we continuosly position the matras and vowel marks and increase the fixed
    // until we reached the end.
    static struct {
	Form form;
	Position position;
    } finalOrder [] = {
	{ Matra, Pre },
	{ Matra, Below },
	{ VowelMark, Below },
	{ StressMark, Below },
	{ Matra, Above },
	{ Matra, Post },
	{ Consonant, None },
	{ Halant, None },
	{ VowelMark, Above },
	{ StressMark, Above },
	{ VowelMark, Post },
	{ (Form)0, None }
    };

//     qDebug("base=%d fixed=%d", base, fixed );
    int toMove = 0;
    while ( fixed < len ) {
// 	qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = 0;
// 		qDebug("moving from %d to %d", i,  to );
		QChar ch = uc[i];
		unsigned short feature = featuresToApply[i];
		for ( int j = i; j > to; j-- ) {
		    uc[j] = uc[j-1];
		    featuresToApply[j] = featuresToApply[j-1];
		}
		uc[to] = ch;
		switch( finalOrder[toMove].position ) {
		case Pre:
// 		    feature |= PreSubstFeature;
		    break;
		case Above:
// 		    feature |= AboveSubstFeature;
		    break;
		case Below:
		    feature |= BelowFormFeature;//|BelowSubstFeature;
		    break;
		case Post:
		    feature |= PostSubstFeature;//|PostFormFeature;
		    break;
		case None:
		case Split:
		    break;
		}
		featuresToApply[to] = feature;
		fixed++;
	    }
	}
	toMove++;
	if ( finalOrder[toMove].form == 0 )
	    break;
    }

    bool halantForm = base < len-1 && (form( uc[base+1] ) == Halant);
    if ( halantForm ) {
	// #### we have to take care this doesn't get applied to Akhant ligatures,
	// but that's currently rather hard (without a bigger rewrite of the open type
	// API (ftx*.c)
	featuresToApply[base] |= HalantFeature;
	featuresToApply[base+1] |= HalantFeature;
    }

    // set the features we need to apply in OT
    int state = form( uc[0] );
    bool lastWasBase = (base == 0);
    if ( state == Consonant )
	featuresToApply[0] |= AkhantFeature|NuktaFeature;

    for ( int i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature|NuktaFeature;
	    break;
	case Halant:
	    if ( state == Nukta || state == Consonant ) {
		// vattu or halant feature
		if ( isRa( uc[i-1] ) && len > 2 ) {
		    if ( !(featuresToApply[i] & RephFeature) ) {
			featuresToApply[i-1] |= BelowFormFeature|VattuFeature;
			featuresToApply[i] |= BelowFormFeature|VattuFeature;
			int j = i-2;
			while ( j >= 0 ) {
			    int f = form( uc[j] );
			    featuresToApply[j] |= VattuFeature;
			    if ( f == Consonant )
				break;
			    j--;
			}
		    }
		}
		else if ( !lastWasBase  ) {
		    if ( state == Nukta )
			featuresToApply[i-2] |= HalfFormFeature;
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case Nukta:
	    if ( state == Consonant ) {
		featuresToApply[i-1] |= NuktaFeature;
		featuresToApply[i] |= NuktaFeature;
	    }
	case StressMark:
	case VowelMark:
	case Matra:
	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    break;
	}
	state = newState;
    }

    for ( int i = 0; i < (int)reordered.length(); i++ ) {
	attributes[i].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[i].clusterStart = FALSE;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}


// vowel matras that have to be split into two parts.
static const unsigned short bengali_o[2]  = { 0x9c7, 0x9be };
static const unsigned short bengali_au[2] = { 0x9c7, 0x9d7 };

static QString bengaliReorder( const QString &string, int start, int end, unsigned short *featuresToApply,
				GlyphAttributes *attributes, bool invalid )
{
    int len = end - start;

    QString reordered = string.mid( start, len );
    if ( invalid ) {
	reordered = QChar( 0x25cc ) + reordered;
	len++;
    }

    // in case mid() returns the whole string!
    reordered.setLength( reordered.length() );

    QChar *uc = (QChar *)reordered.unicode();

    for ( int i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // We can do this rule at the beginning, as it doesn't interact with later operations.
    // Rule 4: split two part matras into parts
    // This could be done better, but works for now.
    for ( int i = 0; i < len; i++ ) {
	const QChar *split = 0;
	if ( uc[i].unicode() == 0x9cb )
	    split = (const QChar *)bengali_o;
	else if ( uc[i].unicode() == 0x9cc )
	    split = (const QChar *)bengali_au;
	if ( split ) {
	    reordered.replace( i, 1, split, 2 );
	    uc = (QChar *)reordered.unicode();
	    len++;
	    break;
	}
    }
//     qDebug("length=%d",  len );

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[0].clusterStart = FALSE;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {
	bool reph = FALSE;
	if ( len > 2 && isRa( uc[0] ) && form( uc[1] ) == Halant ) {
	    reph = TRUE;
// 	    qDebug("Reph");
	}

	// Rule 1: find base consonant
	int lastConsonant = 0;
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		if ( !lastConsonant )
		    lastConsonant = i;
		// ### The MS specs says, that this should be done only if the syllable starts with a reph,
		// but they seem to act differently.
		if ( /*!reph ||*/ !isRa( uc[i] ) ) {
		    base = i;
		    break;
		}
	    }
	}
	if ( reph && base == 0 )
	    base = lastConsonant;

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

	// Rule 2: move halant of base consonant to last one. Only
	// possible if we skipped consonants while finding the base
	if ( lastConsonant != base && form( uc[base+1] ) == Halant ) {
// 	    qDebug("moving halant from %d to %d!", base+1, lastConsonant);
	    QChar halant = uc[base+1];
	    for ( int i = base+1; i < lastConsonant; i++ )
		uc[i] = uc[i+1];
	    uc[lastConsonant] = halant;

	}

	// Rule 3: Move reph to follow post base matra
	if ( reph ) {
	    int toPos = base+1;
	    if ( toPos < len && form( uc[toPos] ) == Nukta )
		toPos++;
	    // doing this twice takes care of split matras.
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
// 	    qDebug("moving reph from %d to %d", 0, toPos );
	    QChar ra = uc[0];
	    QChar halant = uc[1];
	    for ( int i = 2; i < toPos; i++ )
		uc[i-2] = uc[i];
	    uc[toPos-2] = ra;
	    uc[toPos-1] = halant;
	    featuresToApply[toPos-2] |= RephFeature;
	    featuresToApply[toPos-1] |= RephFeature;
	    base -= 2;
	}
    }

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base+1;
    if ( fixed < len && form( uc[fixed] ) == Nukta )
	fixed++;
    if ( fixed < len - 1 && isRa( uc[fixed] ) && form( uc[fixed+1] ) == Halant )
	fixed += 2;


    // we continuosly position the matras and vowel marks and increase the fixed
    // until we reached the end.
    static struct {
	Form form;
	Position position;
    } finalOrder [] = {
	{ Matra, Pre },
	{ Matra, Below },
	{ VowelMark, Below },
	{ StressMark, Below },
	{ Matra, Above },
	{ Matra, Post },
	{ Consonant, None },
	{ Halant, None },
	{ VowelMark, Above },
	{ StressMark, Above },
	{ VowelMark, Post },
	{ (Form)0, None }
    };

//      qDebug("base=%d fixed=%d", base, fixed );
    int toMove = 0;
    while ( fixed < len ) {
//  	qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = 0;
//  		qDebug("moving from %d to %d", i,  to );
		QChar ch = uc[i];
		unsigned short feature = featuresToApply[i];
		for ( int j = i; j > to; j-- ) {
		    uc[j] = uc[j-1];
		    featuresToApply[j] = featuresToApply[j-1];
		}
		uc[to] = ch;
		switch( finalOrder[toMove].position ) {
		case Pre:
// 		    feature |= PreSubstFeature;
		    break;
		case Above:
// 		    feature |= AboveSubstFeature;
		    break;
		case Below:
		    feature |= BelowFormFeature;//|BelowSubstFeature;
		    break;
		case Post:
		    feature |= PostSubstFeature;//|PostFormFeature;
		    break;
		case None:
		    break;
		case Split:
		    break;
		}
		featuresToApply[to] = feature;
		fixed++;
	    }
	}
	toMove++;
	if ( finalOrder[toMove].form == 0 )
	    break;
    }

    bool halantForm = base < len-1 && (form( uc[base+1] ) == Halant);
    if ( halantForm ) {
	// #### we have to take care this doesn't get applied to Akhant ligatures,
	// but that's currently rather hard (without a bigger rewrite of the open type
	// API (ftx*.c)
	featuresToApply[base] |= HalantFeature;
	featuresToApply[base+1] |= HalantFeature;
    }

    // set the features we need to apply in OT
    int state = form( uc[0] );
    bool lastWasBase = (base == 0);
    if ( state == Consonant )
	featuresToApply[0] |= AkhantFeature|NuktaFeature;

    for ( int i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature|NuktaFeature;
	    break;
	case Halant:
	    if ( state == Nukta || state == Consonant ) {
		// vattu or halant feature
		if ( isRa( uc[i-1] ) && len > 2 ) {
		    if ( !(featuresToApply[i] & RephFeature) ) {
			featuresToApply[i-1] |= BelowFormFeature|VattuFeature;
			featuresToApply[i] |= BelowFormFeature|VattuFeature;
			int j = i-2;
			while ( j >= 0 ) {
			    int f = form( uc[j] );
			    featuresToApply[j] |= VattuFeature;
			    if ( f == Consonant )
				break;
			    j--;
			}
		    }
		}
		else if ( !lastWasBase  ) {
		    if ( state == Nukta )
			featuresToApply[i-2] |= HalfFormFeature;
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case Nukta:
	    if ( state == Consonant ) {
		featuresToApply[i-1] |= NuktaFeature;
		featuresToApply[i] |= NuktaFeature;
	    }
	case StressMark:
	case VowelMark:
	case Matra:
	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    break;
	}
	state = newState;
    }

    for ( int i = 0; i < (int)reordered.length(); i++ ) {
	attributes[i].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[i].clusterStart = FALSE;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}


static const unsigned short tamil_o[2]    = { 0xbc6, 0xbbe };
static const unsigned short tamil_oo[2]   = { 0xbc7, 0xbbe };
static const unsigned short tamil_au[2]   = { 0xbc6, 0xbd7 };

static QString tamilReorder( const QString &string, int start, int end, unsigned short *featuresToApply,
				GlyphAttributes *attributes, bool invalid )
{
    int len = end - start;

    QString reordered = string.mid( start, len );
    if ( invalid ) {
	reordered = QChar( 0x25cc ) + reordered;
	len++;
    }

    // in case mid() returns the whole string!
    reordered.setLength( reordered.length() );

    QChar *uc = (QChar *)reordered.unicode();

    // We can do this rule at the beginning, as it doesn't interact with later operations.
    // Rule 4: split two part matras into parts
    // This could be done better, but works for now.
    for ( int i = 0; i < len; i++ ) {
	const QChar *split = 0;
	if ( uc[i].unicode() == 0xbca )
	    split = (const QChar *)tamil_o;
	else if ( uc[i].unicode() == 0xbcb )
	    split = (const QChar *)tamil_oo;
	else if ( uc[i].unicode() == 0xbcc )
	    split = (const QChar *)tamil_au;
	if ( split ) {
	    reordered.replace( i, 1, split, 2 );
	    uc = (QChar *)reordered.unicode();
	    len++;
	    break;
	}
    }

    for ( int i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[0].clusterStart = FALSE;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {

	// Rule 1: find base consonant
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		base = i;
		break;
	    }
	}

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

    }

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base+1;


    // we continuosly position the matras and vowel marks and increase the fixed
    // until we reached the end.
    static struct {
	Form form;
	Position position;
    } finalOrder [] = {
	{ Matra, Pre },
	{ Matra, Below },
	{ VowelMark, Below },
	{ StressMark, Below },
	{ Matra, Above },
	{ Matra, Post },
	{ Consonant, None },
	{ Halant, None },
	{ VowelMark, Above },
	{ StressMark, Above },
	{ VowelMark, Post },
	{ (Form)0, None }
    };

//     qDebug("base=%d fixed=%d", base, fixed );
    int toMove = 0;
    while ( fixed < len ) {
// 	qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = base;
// 		qDebug("moving from %d to %d", i,  to );
		QChar ch = uc[i];
		unsigned short feature = featuresToApply[i];
		for ( int j = i; j > to; j-- ) {
		    uc[j] = uc[j-1];
		    featuresToApply[j] = featuresToApply[j-1];
		}
		uc[to] = ch;
		switch( finalOrder[toMove].position ) {
		case Pre:
// 		    feature |= PreSubstFeature;
		    break;
		case Above:
// 		    feature |= AboveSubstFeature;
		    break;
		case Below:
		    feature |= BelowFormFeature;//|BelowSubstFeature;
		    break;
		case Post:
		    feature |= PostSubstFeature;//|PostFormFeature;
		    break;
		case None:
		    break;
		case Split:
		    break;
		}
		featuresToApply[to] = feature;
		fixed++;
	    }
	}
	toMove++;
	if ( finalOrder[toMove].form == 0 )
	    break;
    }

    bool halantForm = base < len-1 && (form( uc[base+1] ) == Halant);
    if ( halantForm ) {
	// #### we have to take care this doesn't get applied to Akhant ligatures,
	// but that's currently rather hard (without a bigger rewrite of the open type
	// API (ftx*.c)
	featuresToApply[base] |= HalantFeature;
	featuresToApply[base+1] |= HalantFeature;
    }

    // set the features we need to apply in OT
    int state = form( uc[0] );
    bool lastWasBase = (base == 0);
    if ( state == Consonant )
	featuresToApply[0] |= AkhantFeature;

    for ( int i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature;
	    break;
	case Halant:
	    if ( state == Consonant ) {
		if ( !lastWasBase  ) {
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case StressMark:
	case VowelMark:
	case Matra:
	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    break;
	}
	state = newState;
    }

    for ( int i = 0; i < (int)reordered.length(); i++ ) {
	attributes[i].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[i].clusterStart = FALSE;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}





typedef QString (*ReorderFunction)( const QString &, int, int,
				    unsigned short *, GlyphAttributes *, bool );

ReorderFunction reorderFunctions[10] = {
    devanagariReorder,
    bengaliReorder,
    devanagariReorder,
    devanagariReorder,
    devanagariReorder,
    tamilReorder,
    devanagariReorder,
    devanagariReorder,
    devanagariReorder,
    devanagariReorder
};



static QString analyzeSyllables( int script, const QString &string, int from, int length, unsigned short *featuresToApply,
				 GlyphAttributes *attributes ) {
    QString reordered;

    int sstart = from;
    int end = sstart + length;
    int fpos = 0;
    while ( sstart < end ) {
	bool invalid;
	int send = nextSyllableBoundary( script, string, sstart, end, &invalid );
// 	qDebug("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
// 	       invalid ? "true" : "false" );
	QString str = (reorderFunctions[script-QFont::Devanagari])( string, sstart, send, featuresToApply+fpos, attributes+fpos, invalid );
	reordered += str;
	fpos += str.length();

	sstart = send;
    }
    return reordered;
}


static void indic_shape( int script, const QString &string, int from, int len, QScriptItem *item )
{
//     qDebug("QScriptEngineDevanagari::shape()");
    QShapedItem *shaped = item->shaped;

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;
    if ( len > 127 )
	featuresToApply = new unsigned short[ 2*len ];


    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, len * 2 * sizeof( GlyphAttributes ) );

    QString reordered = analyzeSyllables( script, string, from, len, featuresToApply, shaped->glyphAttributes );
    shaped->num_glyphs = reordered.length();

    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, shaped->num_glyphs * sizeof( unsigned short ) );
    int pos = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	if ( shaped->glyphAttributes[i].clusterStart )
	    pos = i;
	shaped->logClusters[i] = pos;
    }

    shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
    int error = item->fontEngine->stringToCMap( reordered.unicode(), shaped->num_glyphs, shaped->glyphs, &shaped->num_glyphs );
    if ( error == QFontEngine::OutOfMemory ) {
	shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
	item->fontEngine->stringToCMap( reordered.unicode(), shaped->num_glyphs, shaped->glyphs, &shaped->num_glyphs );
    }

    QOpenType *openType = item->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Devanagari ) ) {
	((QOpenType *) openType)->apply( QFont::Devanagari, featuresToApply, item, len );
    } else {
	heuristicSetGlyphAttributes( string, from, len, item );
	q_calculateAdvances( item );
    }

    if ( len > 127 )
	delete featuresToApply;
}



// --------------------------------------------------------------------------------------------------------------------------------------------
//
// The script engine jump table
//
// --------------------------------------------------------------------------------------------------------------------------------------------

q_scriptEngine scriptEngines[] = {
	// Latin,
    { basic_shape, basic_attributes },
	// Greek,
    { basic_shape, basic_attributes },
	// Cyrillic,
    { basic_shape, basic_attributes },
	// Armenian,
    { basic_shape, basic_attributes },
	// Georgian,
    { basic_shape, basic_attributes },
	// Runic,
    { basic_shape, basic_attributes },
	// Ogham,
    { basic_shape, basic_attributes },
	// SpacingModifiers,
    { basic_shape, basic_attributes },
	// CombiningMarks,
    { basic_shape, basic_attributes },

	// // Middle Eastern Scripts
	// Hebrew,
    { basic_shape, basic_attributes },
	// Arabic,
    { arabic_shape, arabic_attributes },
	// Syriac,
    { syriac_shape, arabic_attributes },
	// Thaana,
    { basic_shape, basic_attributes },

	// // South and Southeast Asian Scripts
	// Devanagari,
    { indic_shape, basic_attributes },
	// Bengali,
    { indic_shape, basic_attributes },
	// Gurmukhi,
    { indic_shape, basic_attributes },
	// Gujarati,
    { indic_shape, basic_attributes },
	// Oriya,
    { indic_shape, basic_attributes },
	// Tamil,
    { indic_shape, basic_attributes },
	// Telugu,
    { indic_shape, basic_attributes },
	// Kannada,
    { indic_shape, basic_attributes },
	// Malayalam,
    { indic_shape, basic_attributes },
	// Sinhala,
    { indic_shape, basic_attributes },
	// Thai,
    { indic_shape, basic_attributes },
	// Lao,
    { indic_shape, basic_attributes },
	// Tibetan,
    { indic_shape, basic_attributes },
	// Myanmar,
    { indic_shape, basic_attributes },
	// Khmer,
    { indic_shape, basic_attributes },

	// // East Asian Scripts
	// Han,
    { basic_shape, basic_attributes },
	// Hiragana,
    { basic_shape, basic_attributes },
	// Katakana,
    { basic_shape, basic_attributes },
	// Hangul,
    { basic_shape, basic_attributes },
	// Bopomofo,
    { basic_shape, basic_attributes },
	// Yi,
    { basic_shape, basic_attributes },

	// // Additional Scripts
	// Ethiopic,
    { basic_shape, basic_attributes },
	// Cherokee,
    { basic_shape, basic_attributes },
	// CanadianAboriginal,
    { basic_shape, basic_attributes },
	// Mongolian,
    { basic_shape, basic_attributes },

	// // Symbols
	// CurrencySymbols,
    { basic_shape, basic_attributes },
	// LetterlikeSymbols,
    { basic_shape, basic_attributes },
	// NumberForms,
    { basic_shape, basic_attributes },
	// MathematicalOperators,
    { basic_shape, basic_attributes },
	// TechnicalSymbols,
    { basic_shape, basic_attributes },
	// GeometricSymbols,
    { basic_shape, basic_attributes },
	// MiscellaneousSymbols,
    { basic_shape, basic_attributes },
	// EnclosedAndSquare,
    { basic_shape, basic_attributes },
	// Braille,
    { basic_shape, basic_attributes },

	// Unicode,
    { basic_shape, basic_attributes },
};
