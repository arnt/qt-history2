#include "scriptenginearabic.h"

#include "private/qunicodetables_p.h"
#include <stdlib.h>
#include "opentype.h"

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
	if ( isMark( *ch ) ) {
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



void ScriptEngineArabic::charAttributes( const QString &text, int from, int len, CharAttributes *attributes )
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

    d->glyphAttributes = (GlyphAttributes *)realloc( d->glyphAttributes, d->length * sizeof( GlyphAttributes ) );
    d->logClusters = (unsigned short *) realloc( d->logClusters, d->length * sizeof( unsigned short ) );
    QChar *shaped = (QChar *)malloc( d->length * sizeof( QChar ) );

    shapedString( text, from, len, shaped, &d->num_glyphs, (d->analysis.bidiLevel%2),
		  d->glyphAttributes, d->logClusters );

    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( shaped, d->num_glyphs, d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( shaped, d->num_glyphs, d->glyphs, &d->num_glyphs );
    }

    free( shaped );

}


void ScriptEngineArabic::position( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( OpenTypeIface::Arabic ) ) {
	openTypePosition( openType, result );
	return;
    }

    ScriptEngineBasic::position( result );


#if 0
    Offset *advances = result->d->advances;
    Offset *offsets = result->d->offsets;
    qDebug("positioned glyphs:" );
    for ( int i = 0; i < result->d->num_glyphs; i++) {
	qDebug("   ->\tadv=(%d/%d)\tpos=(%d/%d)",
	       advances[i].x, advances[i].y, offsets[i].x, offsets[i].y );
    }
#endif
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

#if 0
    qDebug("before shaping: glyph attributes:" );
    for ( int i = 0; i < result->d->num_glyphs; i++) {
	qDebug("   ->\tmark=%d",
	       result->d->glyphAttributes[i].mark );
    }
#endif

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;

    bool allocated = FALSE;
    if ( d->num_glyphs > 255 ) {
	featuresToApply = (unsigned short *)malloc( d->num_glyphs );
	allocated = TRUE;
    }

    for ( int i = 0; i < d->num_glyphs; i++ )
	featuresToApply[i] = shapeToOpenTypeBit[glyphVariantLogical( text, from + i )];

    ((OpenTypeIface *) openType)->applyGlyphSubstitutions( OpenTypeIface::Arabic, result, featuresToApply );

    if ( allocated )
	free( featuresToApply );
}


void ScriptEngineArabic::openTypePosition( const OpenTypeIface *openType, ShapedItem *result )
{
    ShapedItemPrivate *d = result->d;
    d->offsets = (Offset *) realloc( d->offsets, d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );
    d->advances = (Offset *) realloc( d->advances, d->num_glyphs * sizeof( Offset ) );
    for ( int i = 0; i < d->num_glyphs; i++ ) {
	QGlyphInfo gi = d->fontEngine->boundingBox( d->glyphs[i] );
	d->advances[i].x = gi.xoff;
	d->advances[i].y = gi.yoff;
	// #### not quite correct! should be done after glyph positioning!
	int y = d->offsets[i].y + gi.y;
	d->ascent = QMAX( d->ascent, -y );
	d->descent = QMAX( d->descent, y + gi.height );
    }

    bool positioned = ((OpenTypeIface *) openType)->applyGlyphPositioning( OpenTypeIface::Arabic, result );
#if 0
    qDebug("after positoning: glyph attributes:" );
    for ( int i = 0; i < result->d->num_glyphs; i++) {
	qDebug("   ->\tmark=%d",
	       result->d->glyphAttributes[i].mark );
    }
#endif
    if ( !positioned ) {
// 	qDebug("no open type positioning, using heuristics");
	heuristicPositionMarks( result );
    }


//     qDebug("logClusters:");
//     for ( int i = 0; i < result->d->length; i++ )
// 	qDebug("    %d -> %d", i, result->d->logClusters[i] );
}
