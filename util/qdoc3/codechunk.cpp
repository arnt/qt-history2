/*
  codechunk.cpp
*/

#include <qregexp.h>
#include <qstringlist.h>

#include "codechunk.h"

enum { Other, Alnum, Gizmo, Comma, LParen, RParen, RAngle, Colon };

// entries 128 and above are Other
static const int charCategory[256] = {
    Other,  Other,  Other,  Other,  Other,  Other,  Other,  Other,
    Other,  Other,  Other,  Other,  Other,  Other,  Other,  Other,
    Other,  Other,  Other,  Other,  Other,  Other,  Other,  Other,
    Other,  Other,  Other,  Other,  Other,  Other,  Other,  Other,
//          !       "       #       $       %       &       '
    Other,  Other,  Other,  Other,  Other,  Gizmo,  Gizmo,  Other,
//  (       )       *       +       ,       -       .       /
    LParen, RParen, Gizmo,  Gizmo,  Comma,  Other,  Other,  Gizmo,
//  0       1       2       3       4       5       6       7
    Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  8       9       :       ;       <       =       >       ?
    Alnum,  Alnum,  Colon,  Other,  Other,  Gizmo,  RAngle, Gizmo,
//  @       A       B       C       D       E       F       G
    Other,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  H       I       J       K       L       M       N       O
    Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  P       Q       R       S       T       U       V       W
    Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  X       Y       Z       [       \       ]       ^       _
    Alnum,  Alnum,  Alnum,  Other,  Other,  Other,  Gizmo,  Alnum,
//  `       a       b       c       d       e       f       g
    Other,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  h       i       j       k       l       m       n       o
    Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  p       q       r       s       t       u       v       w
    Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,  Alnum,
//  x       y       z       {       |       }       ~
    Alnum,  Alnum,  Alnum,  LParen, Gizmo,  RParen, Other,  Other
};

static const bool needSpace[8][8] = {
    /*        [      a      +      ,      (       )     >      :  */
    /* [ */ { false, false, false, false, false, true,  false, false },
    /* a */ { false, true,  true,  false, false, true,  false, false },
    /* + */ { false, true,  false, false, false, true,  false, true },
    /* , */ { true,  true,  true,  true,  true,  true,  true,  true },
    /* ( */ { true,  true,  true,  false, true,  false, true,  true },
    /* ) */ { true,  true,  true,  false, true,  true,  true,  true },
    /* > */ { true,  true,  true,  false, true,  true,  true,  false },
    /* : */ { false, false, true,  true,  true,  true,  true,  false }
};

static int category( QChar ch )
{
    return charCategory[(int)ch.toLatin1()];
}

CodeChunk::CodeChunk()
    : s( "" ), bstart( -1 ), blen( 0 ), hspot( -1 )
{
}

CodeChunk::CodeChunk( const QString& str )
    : s( str ), bstart( 0 ), hspot( -1 )
{
    /*
      That's good enough for base class names.
    */
    blen = str.indexOf( QChar('<') );
    if ( blen == -1 )
	blen = str.length();
    b = str.left( blen );
}

CodeChunk::CodeChunk( const CodeChunk& chk )
    : s( chk.s ), b( chk.b ), bstart( chk.bstart ), blen( chk.blen ),
      hspot( chk.hspot )
{
}

CodeChunk& CodeChunk::operator=( const CodeChunk& chk )
{
    s = chk.s;
    b = chk.b;
    bstart = chk.bstart;
    blen = chk.blen;
    hspot = chk.hspot;
    return *this;
}

void CodeChunk::append( const QString& lexeme )
{
    if ( !s.isEmpty() && !lexeme.isEmpty() ) {
	/*
	  Should there be a space or not between the code chunk so far and the
	  new lexeme?
	*/
        int cat1 = category(s.right(1)[0]);
        int cat2 = category(lexeme[0]);
	if ( needSpace[cat1][cat2] )
	    s += QChar( ' ' );
    }
    s += lexeme;
}

void CodeChunk::appendBase( const QString& lexeme )
{
    append( lexeme );

    /*
      The first base is the right one.  If many bases follow each other, they
      form the base together.
    */
    if ( bstart == -1 )
	bstart = s.length() - lexeme.length();
    if ( bstart + blen + lexeme.length() == s.length() ) {
	blen += lexeme.length();
	b = s.mid( bstart, blen );
    }
}

void CodeChunk::appendHotspot()
{
    /*
      The first hotspot is the right one.
    */
    if ( hspot == -1 )
	hspot = s.length();
}

QString CodeChunk::toString() const
{
    return s;
}

QStringList CodeChunk::toPath() const
{
    QString t = s;
    t.remove(QRegExp("<([^<>]|<([^<>]|<[^<>]*>)*>)*>"));
    return t.split("::");
}
