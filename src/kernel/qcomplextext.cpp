/****************************************************************************
** $Id$
**
** Implementation of some internal classes
**
** Created :
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "private/qcomplextext_p.h"

#ifndef QT_NO_COMPLEXTEXT
#include "private/qrichtext_p.h"
#include "private/qfontdata_p.h"
#include "qfontmetrics.h"
#include "qrect.h"

#include <stdlib.h>

// -----------------------------------------------------

/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
QBidiContext::QBidiContext( uchar l, QChar::Direction e, QBidiContext *p, bool o )
    : level(l) , override(o), dir(e)
{
    if ( p )
	p->ref();
    parent = p;
    count = 0;
}

QBidiContext::~QBidiContext()
{
    if( parent && parent->deref() )
	delete parent;
}

static QChar *shapeBuffer = 0;
static int shapeBufSize = 0;

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
static inline const QChar *prevChar( const QString &str, int pos )
{
    //qDebug("leftChar: pos=%d", pos);
    pos--;
    const QChar *ch = str.unicode() + pos;
    while( pos > -1 ) {
	if( !ch->isMark() )
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
	if( !ch->isMark() )
	    return ch;
	// assume it's a transparent char, this might not be 100% correct
	pos++;
	ch++;
    }
    return &QChar::replacement;
}

static inline bool prevVisualCharJoins( const QString &str, int pos)
{
    return (     prevChar( str, pos )->joining() != QChar::OtherJoining );
}

static inline bool nextVisualCharJoins( const QString &str, int pos)
{
    QChar::Joining join = nextChar( str, pos )->joining();
    return ( join == QChar::Dual || join == QChar::Center );
}


QComplexText::Shape QComplexText::glyphVariant( const QString &str, int pos)
{
    // ignores L1 - L3, done in the codec
    QChar::Joining joining = str[pos].joining();
    //qDebug("checking %x, joining=%d", str[pos].unicode(), joining);
    switch ( joining ) {
	case QChar::OtherJoining:
	case QChar::Center:
	    // these don't change shape
	    return XIsolated;
	case QChar::Right:
	    // only rule R2 applies
	    return ( nextVisualCharJoins( str, pos ) ) ? XFinal : XIsolated;
	case QChar::Dual:
	    bool right = nextVisualCharJoins( str, pos );
	    bool left = prevVisualCharJoins( str, pos );
	    //qDebug("dual: right=%d, left=%d", right, left);
	    return ( right ) ? ( left ? XMedial : XFinal ) : ( left ? XInitial : XIsolated );
    }
    return XIsolated;
}

/* and the same thing for logical ordering :)
 */
static inline bool prevLogicalCharJoins( const QString &str, int pos)
{
    return (     nextChar( str, pos )->joining() != QChar::OtherJoining );
}

static inline bool nextLogicalCharJoins( const QString &str, int pos)
{
    QChar::Joining join = prevChar( str, pos )->joining();
    return ( join == QChar::Dual || join == QChar::Center );
}


QComplexText::Shape QComplexText::glyphVariantLogical( const QString &str, int pos)
{
    // ignores L1 - L3, ligatures are job of the codec
    QChar::Joining joining = str[pos].joining();
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

// -------------------------------------------------------------

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

static inline int getShape( const QChar * /* base */, uchar cell, int shape,
			    const QFontMetrics * /* fm */ )
{
    // the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, handle this here
    uint ch = ( cell != 0x49 ) ? arabicUnicodeMapping[cell][0] + shape
	    		       : alefMaksura[shape] ;
    /*
    // we revert to the unshaped glyph in case the shaped version doesn't exist
    if ( fm && !fm->inFont( ch ) ) {
        switch( shape ) {
            case QComplexText::XIsolated:
                break; // try base form
            case QComplexText::XFinal:
                ch -= 1; // try isolated form
                break;
            case QComplexText::XInitial:
                ch += 1; // try medial form
                break;
            case QComplexText::XMedial:
                ch -= 1; // try initial form
                break;
         }
         if ( !fm->inFont( ch ) )
             ch = *base;
    }
    */
    return ch;
}

QString QComplexText::shapedString(const QString& uc, int from, int len, QPainter::TextDirection dir, const QFontMetrics *fm )
{
    if( len < 0 ) {
	len = uc.length() - from;
    } else if( len == 0 ) {
	return QString::null;
    }

    // we have to ignore NSMs at the beginning and add at the end.
    int num = uc.length() - from - len;
    const QChar *ch = uc.unicode() + from + len;
    while ( num > 0 && ch->combiningClass() != 0 ) {
	ch++;
	num--;
	len++;
    }
    ch = uc.unicode() + from;
    while ( len > 0 && ch->combiningClass() != 0 ) {
	ch++;
	len--;
	from++;
    }
    if ( len == 0 ) return QString::null;

    if( !shapeBuffer || len > shapeBufSize ) {
      if( shapeBuffer ) free( (void *) shapeBuffer );
      shapeBuffer = (QChar *) malloc( len*sizeof( QChar ) );
//        delete [] shapeBuffer;
//        shapeBuffer = new QChar[ len + 1];
	shapeBufSize = len;
    }

    int lenOut = 0;
    QChar *data = shapeBuffer;
    if ( dir == QPainter::RTL )
	ch += len - 1;
    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	uchar c = ch->cell();
	if ( r != 0x06 ) {
	    if ( r == 0x20 ) {
	        // remove ZWJ and ZWNJ
		switch ( c ) {
		    case 0x0C: // ZERO WIDTH NONJOINER
		    case 0x0D: // ZERO WIDTH JOINER
			goto skip;
		    default:
			break;
		}
	    }
	    if ( dir == QPainter::RTL && ch->mirrored() )
		*data = ch->mirroredChar();
	    else
		*data = *ch;
	    data++;
	    lenOut++;
	} else {
	    int pos = i + from;
	    if ( dir == QPainter::RTL )
		pos = from + len - 1 - i;
	    int shape = glyphVariantLogical( uc, pos );
	    //qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, arabicUnicodeMapping[ch->cell()][shape]);
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
				//qDebug(" lam of lam-alef ligature");
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
	    map = getShape( ch, c, shape, fm );
	next:
	    *data = map;
	    data++;
	    lenOut++;
	}
    skip:
	if ( dir == QPainter::RTL )
	    ch--;
	else
	    ch++;
    }

    if ( dir == QPainter::Auto && !uc.simpleText() ) {
	return bidiReorderString( QConstString( shapeBuffer, lenOut ).string() );
    }
    if ( dir == QPainter::RTL ) {
	// reverses the non spacing marks to be again after the base char
	QChar *s = shapeBuffer;
	int i = 0;
	while ( i < lenOut ) {
	    if ( s->combiningClass() != 0 ) {
		// non spacing marks
		int clen = 1;
		QChar *ch = s;
		do {
		    ch++;
		    clen++;
		} while ( ch->combiningClass() != 0 );

		int j = 0;
		QChar *cp = s;
		while ( j < clen/2 ) {
		    QChar tmp = *cp;
		    *cp = *ch;
		    *ch = tmp;
		    cp++;
		    ch--;
		    j++;
		}
		s += clen;
		i += clen;
	    } else {
		s++;
		i++;
	    }
	}
    }

    return QConstString( shapeBuffer, lenOut ).string();
}

QChar QComplexText::shapedCharacter( const QString &str, int pos, const QFontMetrics *fm )
{
    const QChar *ch = str.unicode() + pos;
    uchar r = ch->row();
    if ( r != 0x06 ) {
	if ( r == 0x20 ) {
	    // remove ZWJ and ZWNJ
	    switch ( ch->cell() ) {
		case 0x0C: // ZERO WIDTH NONJOINER
		case 0x0D: // ZERO WIDTH JOINER
		    return QChar(0);
		default:
		    break;
	    }
	}
	return *ch;
    } else {
	int shape = glyphVariantLogical( str, pos );
	//qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, arabicUnicodeMapping[ch->cell()][shape]);
	// lam aleph ligatures
	uchar c = ch->cell();
	switch ( c ) {
	    case 0x44: { // lam
		const QChar *nch = nextChar( str, pos );
		if ( nch->row() == 0x06 ) {
		    switch ( nch->cell() ) {
			case 0x22:
			case 0x23:
			case 0x25:
			case 0x27:
			    return QChar(arabicUnicodeLamAlefMapping[nch->cell() - 0x22][shape]);
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
		if ( prevChar( str, pos )->unicode() == 0x0644 )
		    // have a lam alef ligature
		    return QChar(0);
	    default:
		break;
	}
	return QChar( getShape( ch, c, shape, fm ) );
    }
}

QPointArray QComplexText::positionMarks( QFontPrivate *f, const QString &str,
					 int pos, QRect *boundingRect )
{
    int len = str.length();
    int nmarks = 0;
    while ( pos + nmarks < len && str[pos+nmarks +1].combiningClass() > 0 )
	nmarks++;

    if ( !nmarks )
	return QPointArray();

    QChar baseChar = QComplexText::shapedCharacter( str, pos );
    QRect baseRect = f->boundingRect( baseChar );
    int baseOffset = f->textWidth( str, pos, 1 );

    //qDebug( "base char: bounding rect at %d/%d (%d/%d)", baseRect.x(), baseRect.y(), baseRect.width(), baseRect.height() );
    int offset = f->actual.pixelSize / 10 + 1;
    //qDebug("offset = %d", offset );
    QPointArray pa( nmarks );
    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    if ( boundingRect )
	*boundingRect = baseRect;
    for( i = 0; i < nmarks; i++ ) {
	QChar mark = str[pos+i+1];
	unsigned char cmb = mark.combiningClass();
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew and thai.

	    // ### add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 )
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
	    else if ( cmb == 103 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 )
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

	QPoint p;
	QRect markRect = f->boundingRect( mark );
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
	//qDebug( "char=%x combiningClass = %d offset=%d/%d", mark.unicode(), cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	p += QPoint( -baseOffset, 0 );
	attachmentRect |= markRect;
	if ( boundingRect )
	    *boundingRect |= markRect;
	lastCmb = cmb;
	pa.setPoint( i, p );
    }
    return pa;
}

#define BIDI_DEBUG 0 // 2
#if (BIDI_DEBUG >= 1)
#include <iostream>
#endif

static QChar::Direction basicDirection(const QString &str, int start = 0)
{
    int len = str.length();
    int pos = start > len ? len -1 : start;
    const QChar *uc = str.unicode() + pos;
    while( pos < len ) {
	switch( uc->direction() )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return QChar::DirL;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return QChar::DirR;
	default:
	    break;
	}
	++pos;
	++uc;
    }
    if ( start != 0 )
	return basicDirection( str );
    return QChar::DirL;
}

// transforms one line of the paragraph to visual order
// the caller is responisble to delete the returned list of QTextRuns.
QPtrList<QTextRun> *QComplexText::bidiReorderLine( QBidiControl *control, const QString &text, int start, int len,
						   QChar::Direction basicDir )
{
    int last = start + len - 1;
    //printf("doing BiDi reordering from %d to %d!\n", start, last);

    QPtrList<QTextRun> *runs = new QPtrList<QTextRun>;
    runs->setAutoDelete(TRUE);

    QBidiContext *context = control->context;
    if ( !context ) {
	// first line
	if( start != 0 )
	    qDebug( "bidiReorderLine::internal error");
	if( basicDir == QChar::DirR || (basicDir == QChar::DirON && text.isRightToLeft() ) ) {
	    context = new QBidiContext( 1, QChar::DirR );
	    control->status.last = QChar::DirR;
	} else {
	    context = new QBidiContext( 0, QChar::DirL );
	    control->status.last = QChar::DirL;
	}
    }

    QBidiStatus status = control->status;
    QChar::Direction dir = QChar::DirON;

    int sor = start;
    int eor = start;

    int current = start;
    while(current <= last) {
	QChar::Direction dirCurrent;
	if(current == (int)text.length()) {
	    QBidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else if ( current == last ) {
	    dirCurrent = ( basicDir != QChar::DirON ? basicDir : basicDirection( text, current ) );
	} else
	    dirCurrent = text.at(current).direction();


#if (BIDI_DEBUG >= 2)
	cout << "directions: dir=" << dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << " level =" << (int)context->level << endl;
#endif

	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context);
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context);
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirRLO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context, TRUE);
		    dir = QChar::DirR;
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context, TRUE);
		    dir = QChar::DirL;
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		QBidiContext *c = context->parent;
		if(c) {
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    status.last = context->dir;
		    if( context->deref() ) delete context;
		    context = c;
		    if(context->override)
			dir = context->dir;
		    else
			dir = QChar::DirON;
		    status.lastStrong = context->dir;
		}
		break;
	    }

	    // strong types
	case QChar::DirL:
	    if(dir == QChar::DirON)
		dir = QChar::DirL;
	    switch(status.last)
		{
		case QChar::DirL:
		    eor = current; status.eor = QChar::DirL; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(dir != QChar::DirL) {
			//last stuff takes embedding dir
			if( context->dir == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    else
				eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			} else {
			    if(status.eor != QChar::DirL) {
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirL;
			    } else {
				eor = current; status.eor = QChar::DirL; break;
			    }
			}
		    } else {
			eor = current; status.eor = QChar::DirL;
		    }
		default:
		    break;
		}
	    status.lastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(dir == QChar::DirON) dir = QChar::DirR;
	    switch(status.last)
		{
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if( status.eor != QChar::DirR && status.eor != QChar::DirAL ) {
			//last stuff takes embedding dir
			if(context->dir == QChar::DirR || status.lastStrong == QChar::DirR) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			}
		    } else {
			eor = current; status.eor = QChar::DirR;
		    }
		default:
		    break;
		}
	    status.lastStrong = dirCurrent;
	    break;

	    // weak types:

	case QChar::DirNSM:
	    // ### if @sor, set dir to dirSor
	    break;
	case QChar::DirEN:
	    if(status.lastStrong != QChar::DirAL) {
		// if last strong was AL change EN to AL
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirL)
			dir = QChar::DirL;
		    else
			dir = QChar::DirAN;
		}
		switch(status.last)
		    {
		    case QChar::DirET:
			if ( status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL ) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			// fall through
		    case QChar::DirEN:
		    case QChar::DirL:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor; sor = eor; status.eor = QChar::DirEN;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN || dir == QChar::DirAN) {
			    eor = current; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirEN;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirAN;
			    } else {
				eor = current; status.eor = dirCurrent;
			    }
			}
		    default:
			break;
		    }
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(dir == QChar::DirON) dir = QChar::DirAN;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirAN:
		    eor = current; status.eor = QChar::DirAN; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		    runs->append( new QTextRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirAN;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; status.eor = QChar::DirR; break;
		    }
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(status.eor == QChar::DirR) {
			// neutrals go to R
			eor = current - 1;
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirAN;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirAN;
			    dir = QChar::DirAN;
			} else {
			    eor = current; status.eor = dirCurrent;
			}
		    }
		default:
		    break;
		}
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(status.last == QChar::DirEN) {
		dirCurrent = QChar::DirEN;
		eor = current; status.eor = dirCurrent;
		break;
	    }
	    break;

	    // boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
	    // neutrals
	case QChar::DirB:
	    // ### what do we do with newline and paragraph separators that come to here?
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << endl;

	if(current >= (int)text.length()) break;

	// set status.last as needed.
	switch(dirCurrent)
	    {
	    case QChar::DirET:
	    case QChar::DirES:
	    case QChar::DirCS:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		switch(status.last)
		    {
		    case QChar::DirL:
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirEN:
		    case QChar::DirAN:
			status.last = dirCurrent;
			break;
		    default:
			status.last = QChar::DirON;
		    }
		break;
	    case QChar::DirNSM:
	    case QChar::DirBN:
		// ignore these
		break;
	    default:
		status.last = dirCurrent;
	    }

	++current;
    }

#if (BIDI_DEBUG >= 1)
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if ( sor <= eor )
	runs->append( new QTextRun(sor, eor, context, dir) );

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    QTextRun *r = runs->first();
    while ( r ) {
	//printf("level = %d\n", r->level);
	if ( r->level > levelHigh )
	    levelHigh = r->level;
	if ( r->level < levelLow )
	    levelLow = r->level;
	r = runs->next();
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#if (BIDI_DEBUG >= 1)
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    cout << "logical order is:" << endl;
    QPtrListIterator<QTextRun> it2(*runs);
    QTextRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
	cout << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = runs->count() - 1;

    while(levelHigh >= levelLow)
    {
	int i = 0;
	while ( i < count )
	{
	    while(i < count && runs->at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i <= count && runs->at(i)->level >= levelHigh) i++;
	    int end = i-1;

	    if(start != end)
	    {
		//cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    QTextRun *first = runs->take(start+j);
		    QTextRun *last = runs->take(end-j-1);
		    runs->insert(start+j, last);
		    runs->insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }

#if (BIDI_DEBUG >= 1)
    cout << "visual order is:" << endl;
    QPtrListIterator<QTextRun> it3(*runs);
    QTextRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	cout << "    " << r3 << endl;
    }
#endif

    control->setContext( context );
    control->status = status;

    return runs;
}


QString QComplexText::bidiReorderString( const QString &str, QChar::Direction /*basicDir*/ )
{

// ### fix basic direction
    QBidiControl control;
    int lineStart = 0;
    int lineEnd = 0;
    int len = str.length();
    QString visual;
    visual.setUnicode( 0, len );
    QChar *vch = (QChar *)visual.unicode();
    const QChar *ch = str.unicode();
    while( lineStart < len ) {
	lineEnd = lineStart;
	while( *ch != '\n' && lineEnd < len ) {
	    ch++;
	    lineEnd++;
	}
	lineEnd++;
	QPtrList<QTextRun> *runs = bidiReorderLine( &control, str, lineStart, lineEnd - lineStart );

	// reorder the content of the line, and output to visual
	QTextRun *r = runs->first();
	while ( r ) {
	    if(r->level %2) {
		// odd level, need to reverse the string
		int pos = r->stop;
		while(pos >= r->start) {
		    *vch = str[pos];
		    if ( vch->mirrored() )
			*vch = vch->mirroredChar();
		    vch++;
		    pos--;
		}
	    } else {
		int pos = r->start;
		while(pos <= r->stop) {
		    *vch = str[pos];
		    vch++;
		    pos++;
		}
	    }
	    r = runs->next();
	}
	if ( *ch == '\n' ) {
	    *vch = *ch;
	    vch++;
	    ch++;
	    lineEnd++;
	}
	lineStart = lineEnd;
	delete runs;
    }
    return visual;
}

QTextRun::QTextRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir) {
    start = _start;
    stop = _stop;
    if(dir == QChar::DirON) dir = context->dir;

    level = context->level;

    // add level of run (cases I1 & I2)
    if( level % 2 ) {
	if(dir == QChar::DirL || dir == QChar::DirAN)
	    level++;
    } else {
	if( dir == QChar::DirR )
	    level++;
	else if( dir == QChar::DirAN )
	    level += 2;
    }
#if (BIDI_DEBUG >= 1)
    printf("new run: dir=%d from %d, to %d level = %d\n", dir, _start, _stop, level);
#endif
}

#endif //QT_NO_COMPLEXTEXT
