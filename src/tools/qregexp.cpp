/****************************************************************************
** $Id: //depot/qt/main/src/tools/qregexp.cpp#78 $
**
** Implementation of QRegExp class
**
** Created : 950126
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qarray.h"
#include "qbitarray.h"
#include "qcache.h"
#include "qintdict.h"
#include "qmap.h"
#include "qregexp.h"
#include "qstring.h"
#include "qtl.h"
#include "qvector.h"

#include <limits.h>

/*
  WARNING!  Be sure to read qregexp.tex before modifying this file.
*/

/*!
  \class QRegExp qregexp.h

  \brief The QRegExp class provides pattern matching using regular expressions.

  \ingroup tools
  \ingroup misc
  \ingroup shared

  Regular expressions are found in such tools as grep, lex, awk and perl.  They
  help users and programmers accomplish simple tasks such as these:

  <ul plain>
  <li> Validate input:  Check whether or not the user entered a valid email
       address, a valid number, etc.
  <li> Search text:  Look for all Bills and Williams in a Unix password file.
  <li> Search-and-replace text:  Replace all occurrences of <tt>\&</tt> by
       <tt>\&amp;</tt>.
  </ul>

  Regular expressions are a generalization of character strings (\l QString).
  Indeed, the most basic regular expressions are just plain strings that avoid
  certain special characters.  Thus, the regular expression <b>begin</b> means
  precisely the same as the string <tt>begin</tt>.  (We will use bold to
  distinguish regular expressions from ordinary strings.)

  But regular expressions are much more general; they can specify a
  whole family of strings.  For example, <b>Bill|William</b> (read
  "Bill or William") denotes the family consisting of <tt>Bill</tt>
  and <tt>William</tt>, and <b>\w+tion</b> denotes the infinite family
  of all the words ending in <tt>tion</tt> (such as
  <tt>information</tt> and <tt>fffgfgftion</tt>).  Idiomatically, we
  can say that <b>Bill|William</b> matches <tt>Bill</tt>.  It also
  matches <tt>William</tt>, but it doesn't match <tt>Frank</tt>.

  QRegExp's regular expression flavor combines Perl's power and
  Unicode support.  QRegExp also supports the weaker wildcard notation
  (as in <b>*.cpp</b> and <b>chapter[1-5].tex</b>) found in many
  command interpreters.  More on this later.

  Now to the nitty-gritty of regular expression syntax.  You can use
  any of these atoms to match a single character:

  <ul plain>
  <li> <b><em>c</em></b> matches the normal character <tt><em>c</em></tt>
  <li> <b>&#92;c</b> matches the character <tt><em>c</em></tt>, even
       if it is one that QRegExp normally assigns meaning to, such as
       <tt>$</tt> or <tt>&#92;</tt>.  (Note that in C++, this must be
       written as <tt>&#92;&#92;c</tt> since the compiler
       transforms <tt>&#92;&#92;c</tt> into <tt>&#92;c</tt>.)
  <li> <b>&#92;a</b> matches the ASCII bell character (BEL, 0x07)
  <li> <b>\f</b> matches the ASCII form feed character (FF, 0x0C)
  <li> <b>\n</b> matches the ASCII line feed character (LF, 0x0A), also known
       as Unix newline
  <li> <b>\r</b> matches the ASCII carriage return character (CR, 0x0D)
  <li> <b>\t</b> matches the ASCII horizontal tabulation character (HT, 0x09)
  <li> <b>\v</b> matches the ASCII vertical tabulation character (VT, 0x0B)
  <li> <b>\x\e hhhh </b> matches the Unicode character corresponding to the
       hexadecimal number \e hhhh (between 0x0000 and 0xFFFF)
  <li> <b>\0\e ooo </b> (i.e., \ zero <em>ooo</em>) matches the ASCII/Latin-1
       character corresponding to the octal number \e ooo (between
       0 and 0377)
  <li> <b>.</b> (dot) matches any character (including newline)
  <li> <b>\d</b> matches a digit (see QChar::isDigit())
  <li> <b>\D</b> matches a non-digit
  <li> <b>\s</b> matches a whitespace (see QChar::isSpace())
  <li> <b>\S</b> matches a non-whitespace
  <li> <b>\w</b> matches a word character (see QChar::isLetterOrNumber())
  <li> <b>\W</b> matches a non-word character.
  </ul>

  To count backslashes in a string:

  \code
    QRegExp rx( "\\\\" );               // strlen( "\\\\" ) == 2
    int pos = 0;
    int count = 0;
    do {
	pos = rx.search( string, pos ); // search for next backslash
	if ( pos >= 0 ) {               // and count it if it exists
	    pos++;
	    count++;
	}
    } while( pos >= 0 );
  \endcode

  \code
    QRegExp rx( "[1-9][0-9][0-9]" );    // matches "100" up to "999"
    rx.match( "476" );                  // returns TRUE
    rx.match( "1492" );                 // returns FALSE
  \endcode

  The atom <b>[</b>...<b>]</b> that matches one of the characters
  within the brackets.  For instance, <b>[BSD]</b> matches any of
  <tt>B</tt>, <tt>D</tt> and <tt>S</tt>.  Only the following characters
  have special meaning in a set:

  <ul plain>
  <li> <b>&#92;</b> behaves the same way as elsewhere
  <li> <b>^</b> negates the character set so that it matches any character
       not in the set, when it is placed first in the list
  <li> <b>-</b> defines a range of characters
  <li> <b>]</b> ends the character set.
  </ul>

  Thus, <b>[a-zA-Z0-9.\-]</b> matches upper- and lower-case ASCII letters,
  digits, dot and hyphen (but not backslash); whereas <b>[^\s]</b> matches
  anything except white-space (same as <b>\S</b>).

  Let's see some more complex examples.  To match an integer between 100 and
  999, you can use the regular expression <b>[1-9][0-9][0-9]</b>, like this:

  \code
    QRegExp rx( "[1-9][0-9][0-9]" );    // matches "100", "101", ..., "999"
    rx.match( "476" );                  // returns TRUE
    rx.match( "1492" );                 // returns FALSE
  \endcode

  If you are searching for such a number in a text, search() or searchRev() are
  the appropriate choices:

  \code
    QRegExp rx( "[1-9][0-9][0-9]" );    // matches "100", "101", ..., "999"
    rx.search( "503 1511" );            // returns 0 (the position of "503")
    rx.searchRev( "503 1511" );         // returns 5 (the position of "511")
  \endcode

  Notice that <tt>511</tt> is found, even though it's part of a larger number
  (<tt>1511</tt>).  We will soon see how to avoid that.

  Another example:  You want to find lines in your text files that contain a
  trailing space.  Solution:  Use <b>\s\n</b>.

  The following atoms are called anchors.  They make an assertion without
  consuming any input:

  <ul plain>
  <li> <b>\b</b> matches at a word boundary
  <li> <b>\B</b> matches wherever <b>\b</b> doesn't match
  <li> <b>^</b> matches at the beginning of input
  <li> <b>$</b> matches at the end of input
  <li> <b>(?=<em>E</em>)</b> matches if the next characters of input match the
       regular expression <b><em>E</em></b> (positive lookahead)
  <li> <b>(?!<em>E</em>)</b> matches if the next characters of input do not
       match the regular expression <b><em>E</em></b> (negative lookahead).
  </ul>

  Anchors are useful to provide some context for the regular expression.  For
  example, if you want to replace all occurrences of <tt>which</tt> by
  <tt>that</tt>, the regular expression <b>\bwhich\b</b> is a better choice than
  <b>which</b>, because it leaves <tt>whichever</tt> alone.

  Lookaheads push the idea of anchors further.  If you want to changes all
  occurrences of <tt>Donald</tt> to <tt>Don</tt>, except for <tt>Donald
  Duck</tt>, you might write

  \code
    QRegExp rx( "Donald(?!\sDuck)" );
    QString text = "Donald Knuth, Donald Duck and Ronald McDonald";
    text.replace( rx, "Don" );
    // text == "Don Knuth, Donald Duck and Ronald McDon"
  \endcode

  Oops!  It changed <tt>McDonald</tt> into <tt>McDon</tt>.  We can fix that by
  using word-boundary anchors: <b>\bDonald\b(?!\sDuck\b)</b>.

  Sometimes, you might have to match many occurrences in a row of an atom
  <b><em>A</em></b>.  The following quantifiers do just that:

  <ul plain>
  <li> <b><em>A</em>*</b> matches 0 or more occurrences of <b><em>A</em></b>
  <li> <b><em>A</em>?</b> matches 0 or 1 occurrences of <b><em>A</em></b>
  <li> <b><em>A</em>+</b> matches 1 or more occurrences of <b><em>A</em></b>
  <li> <b><em>A</em>{<em>m</em>}</b> matches exactly \e m occurrences of
     <b><em>A</em></b>
  <li> <b><em>A</em>{<em>m</em>,<em>n</em>}</b> matches between \e m and \e n
      occurrences of <b><em>A</em></b>
  <li> <b><em>A</em>{<em>m</em>,}</b> matches at least \e m occurrences of
      <b><em>A</em></b>
  <li> <b><em>A</em>{,<em>n</em>}</b> matches at most \e n occurrences of
      <b><em>A</em></b>.
  </ul>

  The Donald Duck example above fails miserably if <tt>Donald</tt> and
  <tt>Duck</tt> are separated by more than one space.  With quantifiers, we can
  now write <b>\bDonald\b(?!\s+Duck\b)</b>, where <b>\s+</b> means 1 or more
  white-space characters.

  One problem we have so far is that there's no way to apply a quantifier to,
  say, <b>ha</b>.  If we write <b>ha+</b>, only the <b>a</b> is covered by the
  <b>+</b>.  This can be solved by enclosing a regular expression with
  parentheses:

  <ul plain>
  <li> <b>(<em>E</em>)</b> matches the subexpression <b><em>E</em></b> and
      captures the matched text, making it available via back-references and
      capturedTexts() (or cap())
  <li> <b>(?:<em>E</em>)</b> matches the subexpression <b><em>E</em></b>
      without capturing anything.
  </ul>

  To match between 3 and 5 occurrences of <tt>ha</tt>, you can write
  <b>(ha){3,5}</b> or <b>(?:ha){3,5}</b>.  The former is slower, because it
  forces QRegExp to do some book-keeping for capturedTexts().

  Back-references are expressions of the form <b>&#92;<em>X</em></b> that stand
  for the <em>X</em>th substring matched by the <em>X</em>th parenthesized
  subexpression.  Use <b>(.)(.)\2\1</b> to match the palindromes of length 4
  (e.g., <tt>anna</tt>).  Back-references make many things possible, such as
  matching a Fibonacci number of <tt>a</tt>'s (e.g., <tt>aaaaaaaa</tt>).
  <!-- If you doubt this claim, write to Jasmin Blanchette
  (jasmin@trolltech.com). -->

  If <b><em>E</em></b> and <b><em>F</em></b> are two regular
  expressions, you can also write <b><em>E</em>|<em>F</em></b> to
  match either <b><em>E</em></b> or <b><em>F</em></b>.  Thus, a
  mathematician <!-- or a lunatic --> might write
  <b>(min|max|opt)imum</b> to match <tt>minimum</tt>, <tt>maximum</tt>
  or <tt>optimum</tt>.

  When writing regular expressions in C++ code, remember that the C++
  compiler transforms <tt>&#92;</tt> characters.  For example, to match a
  <tt>$</tt> character, you must write <tt>"&#92;&#92;$"</tt> in C++
  source, not <tt>"&#92;$"</tt>.  To match a <tt>&#92;</tt> character,
  you must write <tt>"&#92;&#92;&#92;&#92;"</tt>.

  If you want to know more about regular expressions, read Jeffrey Friedl's
  <em>Mastering Regular Expressions</em> (O'Reilly).

  That's all, folks!  Well, almost.  We haven't seen wildcard mode yet.  In
  wildcard mode, there are only four primitives:

  <ul plain>
  <li> <b><em>c</em></b> matches the character <tt><em>c</em></tt>
  <li> <b>?</b> matches any character
  <li> <b>*</b> matches any sequence of characters
  <li> <b>[<em>...</em>]</b> matches a defined set of characters.
  </ul>

  Here's an example of wildcard syntax:

  \code
    QRegExp rx( "chapter??.*" );
    rx.setWildcard( TRUE );
    rx.match( "chapter01.tex" );        // returns TRUE
    rx.match( "chapter123.doc" );       // returns FALSE
  \endcode

  Here's the same example with regular expression syntax:

  \code
    QRegExp rx( "chapter..\\..*" );
    rx.match( "chapter01.tex" );        // returns TRUE
    rx.match( "chapter123.doc" );       // returns FALSE
  \endcode

  \sa QRegExpValidator QString QStringList
*/

static const int NumBadChars = 128;
#define BadChar( ch ) ( (ch).cell() % NumBadChars )

static const int NoOccurrence = INT_MAX;
static const int EmptyCapture = INT_MAX;
static const int InftyLen = INT_MAX;
static const int InftyRep = 1000;
static const int EOS = -1;

#ifndef QT_NO_REGEXP_OPTIM
static int engCount = 0;
static QArray<int> *noOccurrences = 0;
static QArray<int> *firstOccurrenceAtZero = 0;
#endif

/*
  Merges two QArrays of ints and puts the result into the first one.
*/
static void mergeInto( QArray<int> *a, const QArray<int>& b )
{
    int asize = a->size();
    int bsize = b.size();
    if ( asize == 0 ) {
	*a = b.copy();
#ifndef QT_NO_REGEXP_OPTIM
    } else if ( bsize == 1 && (*a)[asize - 1] < b[0] ) {
	a->resize( asize + 1 );
	(*a)[asize] = b[0];
#endif
    } else if ( bsize >= 1 ) {
	int csize = asize + bsize;
	QArray<int> c( csize );
	int i = 0, j = 0, k = 0;
	while ( i < asize ) {
	    if ( j < bsize ) {
		if ( (*a)[i] == b[j] ) {
		    i++;
		    csize--;
		} else if ( (*a)[i] < b[j] ) {
		    c[k++] = (*a)[i++];
		} else {
		    c[k++] = b[j++];
		}
	    } else {
		memcpy( c.data() + k, (*a).data() + i,
			(asize - i) * sizeof(int) );
		break;
	    }
	}
	c.resize( csize );
	if ( j < bsize )
	    memcpy( c.data() + k, b.data() + j, (bsize - j) * sizeof(int) );
	*a = c;
    }
}

/*
  Merges two disjoint QMaps of (int, int) pairs and puts the result into the
  first one.
*/
static void mergeInto( QMap<int, int> *a, const QMap<int, int>& b )
{
    QMap<int, int>::ConstIterator it;
    for ( it = b.begin(); it != b.end(); ++it )
	a->insert( it.key(), *it );
}

/*
  Returns the value associated to key k in QMap m of (int, int) pairs, or 0 if
  no such value is explicitly present.
*/
static int at( const QMap<int, int>& m, int k )
{
    QMap<int, int>::ConstIterator it = m.find( k );
    if ( it == m.end() )
	return 0;
    else
	return *it;
}

#ifndef QT_NO_REGEXP_WILDCARD
/*
  Translates a wildcard pattern to an equivalent regular expression pattern
  (e.g., *.cpp to .*\.cpp).
*/
static QString wc2rx( const QString& wc )
{
    int wclen = wc.length();
    QString rx = QString::fromLatin1( "" );
    int i = 0;
    while ( i < wclen ) {
	QChar c = wc[i++];
	switch ( c.unicode() ) {
	case '*':
	    rx += QString::fromLatin1( ".*" );
	    break;
	case '?':
	    rx += QChar( '.' );
	    break;
	case '$':
	case '(':
	case ')':
	case '+':
	case '.':
	case '\\':
	case '^':
	case '{':
	case '|':
	case '}':
	    rx += QChar( '\\' );
	    rx += c;
	    break;
	case '[':
	    rx += c;
	    if ( wc[i] == QChar('^') )
		rx += wc[i++];
	    if ( i < wclen ) {
		if ( rx[i] == ']' )
		    rx += wc[i++];
		while ( i < wclen && wc[i] != QChar(']') ) {
		    if ( wc[i] == '\\' )
			rx += QChar( '\\' );
		    rx += wc[i++];
		}
	    }
	    break;
	default:
	    rx += c;
	}
    }
    return rx;
}
#endif

/*
  The class QRegExpEngine encapsulates a modified nondeterministic finite
  automaton (NFA).
*/
class QRegExpEngine : public QShared
{
public:
#ifndef QT_NO_REGEXP_CCLASS
    /*
      The class CharClass represents a set of characters, such as can be found
      in regular expressions (e.g., [a-z] denotes the set {a, b, ..., z}).
    */
    class CharClass
    {
    public:
	CharClass();
	CharClass( const CharClass& cc ) { operator=( cc ); }

	CharClass& operator=( const CharClass& cc );

	void clear();
	bool negative() const { return n; }
	void setNegative( bool negative );
	void addCategories( int cats );
	void addRange( ushort from, ushort to );
	void addSingleton( ushort ch ) { addRange( ch, ch ); }

	bool in( QChar ch ) const;
#ifndef QT_NO_REGEXP_OPTIM
	const QArray<int>& firstOccurrence() const { return occ1; }
#endif

#if defined(QT_DEBUG)
	void dump() const;
#endif

    private:
	/*
	  The struct Range represents a range of characters (e.g., [0-9] denotes
	  range 48 to 57).
	*/
	struct Range
	{
	    ushort from; // 48
	    ushort to; // 57
	};

	int c; // character classes
	QArray<Range> r; // character ranges
	bool n; // negative?
#ifndef QT_NO_REGEXP_OPTIM
	QArray<int> occ1; // first-occurrence array
#endif
    };
#else
    struct CharClass
    {
	int x; // dummy

#ifndef QT_NO_REGEXP_OPTIM
	const QArray<int>& firstOccurrence() const {
	    return *firstOccurrenceAtZero;
	}
#endif
    };
#endif

    QRegExpEngine( bool caseSensitive ) { setup( caseSensitive ); }
    QRegExpEngine( const QString& rx, bool caseSensitive );
#ifndef QT_NO_REGEXP_OPTIM
    ~QRegExpEngine();
#endif

    bool isValid() const { return valid; }
    bool caseSensitive() const { return cs; }
    int numCaptures() const { return realncap; }
    QArray<int> match( const QString& str, int pos, bool minimal,
		       bool oneTest );
    int matchedLength() const { return mmMatchedLen; }

    int createState( QChar ch );
    int createState( const CharClass& cc );
#ifndef QT_NO_REGEXP_BACKREF
    int createState( int bref );
#endif

    void addCatTransitions( const QArray<int>& from, const QArray<int>& to );
#ifndef QT_NO_REGEXP_CAPTURE
    void addPlusTransitions( const QArray<int>& from, const QArray<int>& to,
			     int atom );
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
    int anchorAlternation( int a, int b );
    int anchorConcatenation( int a, int b );
#else
    int anchorAlternation( int a, int b ) { return a & b; }
    int anchorConcatenation( int a, int b ) { return a | b; }
#endif
    void addAnchors( int from, int to, int a );

#ifndef QT_NO_REGEXP_OPTIM
    void setupGoodStringHeuristic( int earlyStart, int lateStart,
				   const QString& str );
    void setupBadCharHeuristic( int minLen, const QArray<int>& firstOcc );
    void heuristicallyChooseHeuristic();
#endif

#if defined(QT_DEBUG)
    void dump() const;
#endif

private:
    enum { CharClassBit = 0x10000, BackRefBit = 0x20000 };

    /*
      The struct State represents one state in a modified NFA.  The input
      characters matched are stored in the state instead of on the transitions,
      something possible for an automaton constructed from a regular expression.
    */
    struct State
    {
#ifndef QT_NO_REGEXP_CAPTURE
	int atom; // which atom does this state belong to?
#endif
	int match; // what does it match? (see CharClassBit and BackRefBit)
	QArray<int> outs; // out-transitions
	QMap<int, int> *reenter; // atoms reentered when transiting out
	QMap<int, int> *anchors; // anchors met when transiting out

#ifndef QT_NO_REGEXP_CAPTURE
	State( int a, int m )
	    : atom( a ), match( m ), reenter( 0 ), anchors( 0 ) { }
#else
	State( int m )
	    : match( m ), reenter( 0 ), anchors( 0 ) { }
#endif
	~State() { delete reenter; delete anchors; }
    };

#ifndef QT_NO_REGEXP_LOOKAHEAD
    /*
      The struct Lookahead represents a lookahead a la Perl (e.g., (?=foo) and
      (?!bar)).
    */
    struct Lookahead
    {
	QRegExpEngine *eng; // NFA representing the embedded regular expression
	bool neg; // negative lookahead?

	Lookahead( QRegExpEngine *eng0, bool neg0 )
	    : eng( eng0 ), neg( neg0 ) { }
	~Lookahead() { delete eng; }
    };
#endif

#ifndef QT_NO_REGEXP_CAPTURE
    /*
      The struct Atom represents one node in the hierarchy of regular expression
      atoms.
    */
    struct Atom
    {
	int parent; // index of parent in array of atoms
	int capture; // index of capture, from 1 to ncap
    };
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
    /*
      The struct AnchorAlternation represents a pair of anchors with OR
      semantics.
    */
    struct AnchorAlternation
    {
	int a; // this anchor...
	int b; // ...or this one
    };
#endif

    enum { InitialState = 0, FinalState = 1 };
    void setup( bool caseSensitive );
    int setupState( int match );

    /*
      Let's hope that 13 lookaheads and 14 back-references are enough.
     */
    enum { MaxLookaheads = 13, MaxBackRefs = 14 };
    enum { Anchor_Dollar = 0x00000001, Anchor_Caret = 0x00000002,
	   Anchor_Word = 0x00000004, Anchor_NonWord = 0x00000008,
	   Anchor_FirstLookahead = 0x00000010,
	   Anchor_BackRef1Empty = Anchor_FirstLookahead << MaxLookaheads,
	   Anchor_BackRef0Empty = Anchor_BackRef1Empty >> 1,
	   Anchor_Alternation = Anchor_BackRef1Empty << MaxBackRefs,

	   Anchor_LookaheadMask = ( Anchor_FirstLookahead - 1 ) ^
		   ( (Anchor_FirstLookahead << MaxLookaheads) - 1 ) };
#ifndef QT_NO_REGEXP_CAPTURE
    int startAtom( bool capture );
    void finishAtom( int atom ) { cf = f[atom].parent; }
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
    int addLookahead( QRegExpEngine *eng, bool negative );
#endif

#ifndef QT_NO_REGEXP_CAPTURE
    bool isBetterCapture( const int *begin1, const int *end1, const int *begin2,
			  const int *end2 );
#endif
    bool testAnchor( int i, int a, const int *capBegin );

#ifndef QT_NO_REGEXP_OPTIM
    bool goodStringMatch();
    bool badCharMatch();
#else
    bool bruteMatch();
#endif
    bool matchHere();

    QVector<State> s; // array of states
    int ns; // number of states
#ifndef QT_NO_REGEXP_CAPTURE
    QArray<Atom> f; // atom hierarchy
    int nf; // number of atoms
    int cf; // current atom
#endif
    int realncap; // number of captures, seen from the outside
    int ncap; // number of captures, seen from the inside
#ifndef QT_NO_REGEXP_CCLASS
    QVector<CharClass> cl; // array of character classes
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
    QVector<Lookahead> ahead; // array of lookaheads
#endif
#ifndef QT_NO_REGEXP_ANCHOR_ALT
    QArray<AnchorAlternation> aa; // array of (a, b) pairs of anchors
#endif
#ifndef QT_NO_REGEXP_OPTIM
    bool caretAnchored; // does the regexp start with ^?
#endif
    bool valid; // is the regular expression valid?
    bool cs; // case sensitive?
#ifndef QT_NO_REGEXP_BACKREF
    int nbrefs; // number of back-references
#endif

#ifndef QT_NO_REGEXP_OPTIM
    bool useGoodStringHeuristic; // use goodStringMatch? otherwise badCharMatch

    int goodEarlyStart; // the index where goodStr can first occur in a match
    int goodLateStart; // the index where goodStr can last occur in a match
    QString goodStr; // the string that any match has to contain

    int minl; // the minimum length of a match
    QArray<int> occ1; // first-occurrence array
#endif

    /*
      The class Box is an abstraction for a regular expression fragment.  It can
      also be seen as one node in the syntax tree of a regular expression with
      synthetized attributes.

      It's interface is ugly for performance reasons.
    */
    class Box
    {
    public:
	Box( QRegExpEngine *engine );
	Box( const Box& b ) { operator=( b ); }

	Box& operator=( const Box& b );

	void clear() { operator=(Box(eng)); }
	void set( QChar ch );
	void set( const CharClass& cc );
#ifndef QT_NO_REGEXP_BACKREF
	void set( int bref );
#endif

	void cat( const Box& b );
	void orx( const Box& b );
	void plus( int atom );
	void opt();
	void catAnchor( int a );
#ifndef QT_NO_REGEXP_OPTIM
	void setupHeuristics();
#endif

#if defined(QT_DEBUG)
	void dump() const;
#endif

    private:
	void addAnchorsToEngine( const Box& to ) const;

	QRegExpEngine *eng; // the automaton under construction
	QArray<int> ls; // the left states (firstpos)
	QArray<int> rs; // the right states (lastpos)
	QMap<int, int> lanchors; // the left anchors
	QMap<int, int> ranchors; // the right anchors
	int skipanchors; // the anchors to match if the box is skipped

#ifndef QT_NO_REGEXP_OPTIM
	int earlyStart; // the index where str can first occur
	int lateStart; // the index where str can last occur
	QString str; // a string that has to occur in any match
	QString leftStr; // a string occurring at the left of this box
	QString rightStr; // a string occurring at the right of this box
	int maxl; // the maximum length of this box (possibly InftyLen)
#endif

	int minl; // the minimum length of this box
#ifndef QT_NO_REGEXP_OPTIM
	QArray<int> occ1; // first-occurrence array
#endif
    };
    friend class Box;

    /*
      This is the lexical analyzer for regular expressions.
    */
    enum { Tok_Eos, Tok_Dollar, Tok_LeftParen, Tok_MagicLeftParen,
	   Tok_PosLookahead, Tok_NegLookahead, Tok_RightParen, Tok_CharClass,
	   Tok_Caret, Tok_Quantifier, Tok_Bar, Tok_Word, Tok_NonWord,
	   Tok_Char = 0x10000, Tok_BackRef = 0x20000 };
    int getChar();
    int getEscape();
#ifndef QT_NO_REGEXP_INTERVAL
    int getRep( int def );
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
    void skipChars( int n );
#endif
    void startTokenizer( const QChar *rx, int len );
    int getToken();

    const QChar *yyIn; // a pointer to the input regular expression pattern
    int yyPos0; // the position of yyTok in the input pattern
    int yyPos; // the position of the next character to read
    int yyLen; // the length of yyIn
    int yyCh; // the last character read
    CharClass *yyCharClass; // attribute for Tok_CharClass tokens
    int yyMinRep; // attribute for Tok_Quantifier
    int yyMaxRep; // ditto
    bool yyError; // syntax error or overflow during parsing?

    /*
      This is the syntactic analyzer for regular expressions.
    */
    int parse( const QChar *rx, int len );
    void parseAtom( Box *box );
    void parseFactor( Box *box );
    void parseTerm( Box *box );
    void parseExpression( Box *box );

    int yyTok; // the last token read
    bool yyMayCapture; // set this to FALSE to disable capturing

    /*
      This is the engine state during matching.
    */
    const QString *mmStr; // a pointer to the input QString
    const QChar *mmIn; // a pointer to the input string data
    int mmPos; // the current position in the string
    int mmLen; // the length of the input string
    bool mmMinimal; // minimal matching?
    QArray<int> mmCaptured; // an array of pairs (start, len)
    QArray<int> mmCapturedNoMatch; // an array of pairs (-1, -1)
    QArray<int> mmBigArray; // big QArray<int> array
    int *mmInNextStack; // is state is mmNextStack?
    int *mmCurStack; // stack of current states
    int *mmNextStack; // stack of next states
    int *mmCurCapBegin; // start of current states' captures
    int *mmNextCapBegin; // start of next states' captures
    int *mmCurCapEnd; // end of current states' captures
    int *mmNextCapEnd; // end of next states' captures
    int *mmTempCapBegin; // start of temporary captures
    int *mmTempCapEnd; // end of temporary captures
    int *mmCapBegin; // start of captures for a next state
    int *mmCapEnd; // end of captures for a next state
    int *mmSlideTab; // bump-along slide table for bad-character heuristic
    int mmSlideTabSize; // size of slide table
#ifndef QT_NO_REGEXP_BACKREF
    QIntDict<int> mmSleeping; // dictionary of back-reference sleepers
#endif
    int mmMatchedLen; // length of match or of matched string for partial match
};

QRegExpEngine::QRegExpEngine( const QString& rx, bool caseSensitive )
#ifndef QT_NO_REGEXP_BACKREF
    : mmSleeping( 101 )
#endif
{
    setup( caseSensitive );
    valid = ( parse(rx.unicode(), rx.length()) == (int) rx.length() );
}

#ifndef QT_NO_REGEXP_OPTIM
QRegExpEngine::~QRegExpEngine()
{
    if ( --engCount == 0 ) {
	delete noOccurrences;
	noOccurrences = 0;
	delete firstOccurrenceAtZero;
	firstOccurrenceAtZero = 0;
    }
}
#endif

/*
  Tries to match in str and returns an array of (begin, length) pairs for
  captured text.  If there is no match, all pairs are (-1, -1).
*/
QArray<int> QRegExpEngine::match( const QString& str, int pos, bool minimal,
				  bool oneTest )
{
    mmStr = &str;
    mmIn = str.unicode();
    if ( mmIn == 0 )
	mmIn = &QChar::null;
    mmPos = pos;
    mmLen = str.length();
    mmMinimal = minimal;
    mmMatchedLen = -1;

    bool matched = FALSE;
    if ( valid && mmPos >= 0 && mmPos <= mmLen ) {
#ifndef QT_NO_REGEXP_OPTIM
	if ( mmPos <= mmLen - minl ) {
	    if ( caretAnchored || oneTest )
		matched = matchHere();
	    else if ( useGoodStringHeuristic )
		matched = goodStringMatch();
	    else
		matched = badCharMatch();
	}
#else
	matched = oneTest ? matchHere() : bruteMatch();
#endif
    }

    if ( matched ) {
	mmCaptured.detach();
	mmCaptured[0] = mmPos;
	mmCaptured[1] = mmMatchedLen;
	for ( int j = 0; j < realncap; j++ ) {
	    int len = mmCapEnd[j] - mmCapBegin[j];
	    mmCaptured[2 + 2 * j] = len > 0 ? mmPos + mmCapBegin[j] : 0;
	    mmCaptured[2 + 2 * j + 1] = len;
	}
	return mmCaptured;
    } else {
	return mmCapturedNoMatch;
    }
}

/*
  The three following functions add one state to the automaton and return the
  number of the state.
*/

int QRegExpEngine::createState( QChar ch )
{
    return setupState( ch.unicode() );
}

int QRegExpEngine::createState( const CharClass& cc )
{
#ifndef QT_NO_REGEXP_CCLASS
    int n = cl.size();
    cl.resize( n + 1 );
    cl.insert( n, new CharClass(cc) );
    return setupState( CharClassBit | n );
#else
    Q_UNUSED( cc );
    return setupState( CharClassBit );
#endif
}

#ifndef QT_NO_REGEXP_BACKREF
int QRegExpEngine::createState( int bref )
{
    if ( bref > nbrefs ) {
	nbrefs = bref;
	if ( nbrefs > MaxBackRefs ) {
	    yyError = TRUE;
	    return 0;
	}
    }
    return setupState( BackRefBit | bref );
}
#endif

/*
  The two following functions add a transition between all pairs of states
  (i, j) where i is fond in from, and j is found in to.

  Cat-transitions are distinguished from plus-transitions for capturing.
*/

void QRegExpEngine::addCatTransitions( const QArray<int>& from,
				       const QArray<int>& to )
{
    for ( int i = 0; i < (int) from.size(); i++ ) {
	State *st = s[from[i]];
	mergeInto( &st->outs, to );
    }
}

#ifndef QT_NO_REGEXP_CAPTURE
void QRegExpEngine::addPlusTransitions( const QArray<int>& from,
					const QArray<int>& to, int atom )
{
    for ( int i = 0; i < (int) from.size(); i++ ) {
	State *st = s[from[i]];
	QArray<int> oldOuts = st->outs.copy();
	mergeInto( &st->outs, to );
	if ( f[atom].capture >= 0 ) {
	    if ( st->reenter == 0 )
		st->reenter = new QMap<int, int>;
	    for ( int j = 0; j < (int) to.size(); j++ ) {
		if ( !st->reenter->contains(to[j]) &&
		     oldOuts.bsearch(to[j]) < 0 )
		    st->reenter->insert( to[j], atom );
	    }
	}
    }
}
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
/*
  Returns an anchor that means a OR b.
*/
int QRegExpEngine::anchorAlternation( int a, int b )
{
    if ( ((a & b) == a || (a & b) == b) && ((a | b) & Anchor_Alternation) == 0 )
	return a & b;

    int n = aa.size();
    aa.resize( n + 1 );
    aa[n].a = a;
    aa[n].b = b;
    return Anchor_Alternation | n;
}

/*
  Returns an anchor that means a AND b.
*/
int QRegExpEngine::anchorConcatenation( int a, int b )
{
    if ( ((a | b) & Anchor_Alternation) == 0 )
	return a | b;
    if ( (b & Anchor_Alternation) != 0 )
	qSwap( a, b );
    int aprime = anchorConcatenation( aa[a ^ Anchor_Alternation].a, b );
    int bprime = anchorConcatenation( aa[a ^ Anchor_Alternation].b, b );
    return anchorAlternation( aprime, bprime );
}
#endif

/*
  Adds anchor a on a transition caracterised by its from state and its to state.
*/
void QRegExpEngine::addAnchors( int from, int to, int a )
{
    State *st = s[from];
    if ( st->anchors == 0 )
	st->anchors = new QMap<int, int>;
    if ( st->anchors->contains(to) )
	a = anchorAlternation( (*st->anchors)[to], a );
    st->anchors->insert( to, a );
}

#ifndef QT_NO_REGEXP_OPTIM
/*
  The two following functions provide the engine with the information needed by
  its matching heuristics.
*/

void QRegExpEngine::setupGoodStringHeuristic( int earlyStart, int lateStart,
					      const QString& str )
{
    goodEarlyStart = earlyStart;
    goodLateStart = lateStart;
    goodStr = cs ? str : str.lower();
}

void QRegExpEngine::setupBadCharHeuristic( int minLen,
					   const QArray<int>& firstOcc )
{
    minl = minLen;
    occ1 = cs ? firstOcc : *firstOccurrenceAtZero;
}

/*
  This function chooses between the good-string and the bad-character
  heuristics.  It computes two scores and chooses the heuristic with the highest
  score.

  Here are some common-sense constraints on the scores that should be respected
  if the formulas are ever modified:  (1) If goodStr is empty, the good-string
  heuristic scores 0.  (2) If the search is case insensitive, the good-string
  heuristic should be used, unless it scores 0.  (Case insensitivity
  turns all entries of occ1 to 0.)  (3) If (goodLateStart - goodEarlyStart) is
  big, the good-string heuristic should score less.
*/
void QRegExpEngine::heuristicallyChooseHeuristic()
{
    int i;

    if ( minl == 0 )
	return;

    /*
      Magic formula:  The good string has to constitute a good proportion of the
      minimum-length string, and appear at a more-or-less known index.
    */
    int goodStringScore = ( 64 * goodStr.length() / minl ) -
			  ( goodLateStart - goodEarlyStart );

    /*
      Less magic formula:  We pick a couple of characters at random, and check
      whether they are good or bad.
    */
    int badCharScore = 0;
    int step = QMAX( 1, NumBadChars / 32 );
    for ( i = 1; i < NumBadChars; i += step ) {
	if ( occ1[i] == NoOccurrence )
	    badCharScore += minl;
	else
	    badCharScore += occ1[i];
    }
    badCharScore /= minl;

    useGoodStringHeuristic = ( goodStringScore > badCharScore );
}
#endif

#if defined(QT_DEBUG)
void QRegExpEngine::dump() const
{
    int i, j;
    qDebug( "Case %ssensitive engine", cs ? "" : "in" );
    qDebug( "  States" );
    for ( i = 0; i < ns; i++ ) {
	qDebug( "  %d%s", i,
		i == InitialState ? " (initial)" :
		i == FinalState ? " (final)" : "" );
#ifndef QT_NO_REGEXP_CAPTURE
	qDebug( "    in atom %d", s[i]->atom );
#endif
	int m = s[i]->match;
	if ( (m & CharClassBit) != 0 ) {
	    qDebug( "    match character class %d", m ^ CharClassBit );
#ifndef QT_NO_REGEXP_CCLASS
	    cl[m ^ CharClassBit]->dump();
#else
	    qDebug( "    negative character class" );
#endif
	} else if ( (m & BackRefBit) != 0 ) {
	    qDebug( "    match back-reference %d", m ^ BackRefBit );
	} else if ( m >= 0x20 && m <= 0x7e ) {
	    qDebug( "    match 0x%.4x (%c)", m, m );
	} else {
	    qDebug( "    match 0x%.4x", m );
	}
	for ( j = 0; j < (int) s[i]->outs.size(); j++ ) {
	    int next = s[i]->outs[j];
	    qDebug( "    -> %d", next );
	    if ( s[i]->reenter != 0 && s[i]->reenter->contains(next) )
		qDebug( "       [reenter %d]", (*s[i]->reenter)[next] );
	    if ( s[i]->anchors != 0 && at(*s[i]->anchors, next) != 0 )
		qDebug( "       [anchors 0x%.8x]", (*s[i]->anchors)[next] );
	}
    }
#ifndef QT_NO_REGEXP_CAPTURE
    if ( nf > 0 ) {
	qDebug( "  Atom    Parent  Capture" );
	for ( i = 0; i < nf; i++ )
	    qDebug( "  %6d  %6d  %6d", i, f[i].parent, f[i].capture );
    }
#endif
#ifndef QT_NO_REGEXP_ANCHOR_ALT
    for ( i = 0; i < (int) aa.size(); i++ )
	qDebug( "  Anchor alternation 0x%.8x: 0x%.8x 0x%.9x", i, aa[i].a,
		aa[i].b );
#endif
}
#endif

void QRegExpEngine::setup( bool caseSensitive )
{
#ifndef QT_NO_REGEXP_OPTIM
    if ( engCount++ == 0 ) {
	noOccurrences = new QArray<int>( NumBadChars );
	firstOccurrenceAtZero = new QArray<int>( NumBadChars );
	noOccurrences->fill( NoOccurrence );
	firstOccurrenceAtZero->fill( 0 );
    }
#endif
    s.setAutoDelete( TRUE );
    s.resize( 32 );
    ns = 0;
#ifndef QT_NO_REGEXP_CAPTURE
    f.resize( 32 );
    nf = 0;
    cf = -1;
#endif
    realncap = 0;
    ncap = 0;
#ifndef QT_NO_REGEXP_CCLASS
    cl.setAutoDelete( TRUE );
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
    ahead.setAutoDelete( TRUE );
#endif
#ifndef QT_NO_REGEXP_OPTIM
    caretAnchored = TRUE;
#endif
    valid = FALSE;
    cs = caseSensitive;
#ifndef QT_NO_REGEXP_BACKREF
    nbrefs = 0;
#endif
#ifndef QT_NO_REGEXP_OPTIM
    useGoodStringHeuristic = FALSE;
    minl = 0;
    occ1 = *firstOccurrenceAtZero;
#endif
    mmCapturedNoMatch.fill( -1, 2 );
}

int QRegExpEngine::setupState( int match )
{
    if ( (ns & (ns + 1)) == 0 && ns + 1 >= (int) s.size() )
	s.resize( (ns + 1) << 1 );
#ifndef QT_NO_REGEXP_CAPTURE
    s.insert( ns, new State(cf, match) );
#else
    s.insert( ns, new State(match) );
#endif
    return ns++;
}

#ifndef QT_NO_REGEXP_CAPTURE
/*
  Functions startAtom() and finishAtom() should be called to delimit atoms.
  When a state is created, it is assigned to the current atom.  The information
  is later used for capturing.
*/
int QRegExpEngine::startAtom( bool capture )
{
    if ( (nf & (nf + 1)) == 0 && nf + 1 >= (int) f.size() )
	f.resize( (nf + 1) << 1 );
    f[nf].parent = cf;
    cf = nf++;
    f[cf].capture = capture ? ncap++ : -1;
    return cf;
}
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
/*
  Creates a lookahead anchor.
*/
int QRegExpEngine::addLookahead( QRegExpEngine *eng, bool negative )
{
    int n = ahead.size();
    if ( n == MaxLookaheads ) {
	yyError = TRUE;
	return 0;
    }
    ahead.resize( n + 1 );
    ahead.insert( n, new Lookahead(eng, negative) );
    return Anchor_FirstLookahead << n;
}
#endif

#ifndef QT_NO_REGEXP_CAPTURE
/*
  We want the longest leftmost captures.
*/
bool QRegExpEngine::isBetterCapture( const int *begin1, const int *end1,
				     const int *begin2, const int *end2 )
{
    for ( int i = 0; i < ncap; i++ ) {
	int delta = begin2[i] - begin1[i]; // it has to start early...
	if ( delta == 0 )
	    delta = end1[i] - end2[i]; // ...and end late (like a party)

	if ( delta != 0 )
	    return delta > 0;
    }
    return FALSE;
}
#endif

/*
  Returns TRUE if anchor a matches at position mmPos + i in the input string,
  otherwise FALSE.
*/
bool QRegExpEngine::testAnchor( int i, int a, const int *capBegin )
{
    int j;

#ifndef QT_NO_REGEXP_ANCHOR_ALT
    if ( (a & Anchor_Alternation) != 0 ) {
	return testAnchor( i, aa[a ^ Anchor_Alternation].a, capBegin ) ||
	       testAnchor( i, aa[a ^ Anchor_Alternation].b, capBegin );
    }
#endif

    if ( (a & Anchor_Caret) != 0 ) {
	if ( mmPos + i != 0 )
	    return FALSE;
    }
    if ( (a & Anchor_Dollar) != 0 ) {
	if ( mmPos + i != mmLen )
	    return FALSE;
    }
#ifndef QT_NO_REGEXP_ESCAPE
    if ( (a & (Anchor_Word | Anchor_NonWord)) != 0 ) {
	bool before = FALSE, after = FALSE;
	if ( mmPos + i != 0 )
	    before = mmIn[mmPos + i - 1].isLetterOrNumber();
	if ( mmPos + i != mmLen )
	    after = mmIn[mmPos + i].isLetterOrNumber();
	if ( (a & Anchor_Word) != 0 && (before == after) )
	    return FALSE;
	if ( (a & Anchor_NonWord) != 0 && (before != after) )
	    return FALSE;
    }
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
    bool catchx = TRUE;

    if ( (a & Anchor_LookaheadMask) != 0 ) {
	QConstString cstr = QConstString( (QChar *) mmIn + mmPos + i,
					   mmLen - mmPos - i );
	for ( j = 0; j < (int) ahead.size(); j++ ) {
	    if ( (a & (Anchor_FirstLookahead << j)) != 0 ) {
		catchx = ( ahead[j]->eng->match(cstr.string(), 0, TRUE,
						TRUE)[0] == 0 );
		if ( catchx == ahead[j]->neg )
		    return FALSE;
	    }
	}
    }
#endif
#ifndef QT_NO_REGEXP_CAPTURE
#ifndef QT_NO_REGEXP_BACKREF
    for ( j = 0; j < nbrefs; j++ ) {
	if ( (a & (Anchor_BackRef1Empty << j)) != 0 ) {
	    if ( capBegin[j] != EmptyCapture )
		return FALSE;
	}
    }
#endif
#endif
    return TRUE;
}

#ifndef QT_NO_REGEXP_OPTIM
/*
  The three following functions are what Jeffrey Friedl would call transmissions
  (or bump-alongs).  Using one or the other should make no difference, except
  in performance.
*/

bool QRegExpEngine::goodStringMatch()
{
    int k = mmPos + goodEarlyStart;

    while ( (k = mmStr->find(goodStr, k, cs)) != -1 ) {
	int from = k - goodLateStart;
	int to = k - goodEarlyStart;
	if ( from > mmPos )
	    mmPos = from;

	while ( mmPos <= to ) {
	    if ( matchHere() )
		return TRUE;
	    mmPos++;
	}
	k++;
    }
    return FALSE;
}

bool QRegExpEngine::badCharMatch()
{
    int slideHead = 0;
    int slideNext = 0;
    int i;
    int lastPos = mmLen - minl;
    memset( mmSlideTab, 0, mmSlideTabSize * sizeof(int) );

    /*
      Set up the slide table, used for the bad-character heuristic, using
      the table of first occurrence of each character.
    */
    for ( i = 0; i < minl; i++ ) {
	int sk = occ1[BadChar(mmIn[mmPos + i])];
	if ( sk == NoOccurrence )
	    sk = i + 1;
	if ( sk > 0 ) {
	    int k = i + 1 - sk;
	    if ( k < 0 ) {
		sk = i + 1;
		k = 0;
	    }
	    if ( sk > mmSlideTab[k] )
		mmSlideTab[k] = sk;
	}
    }

    if ( mmPos > lastPos )
	return FALSE;

    while ( TRUE ) {
	if ( ++slideNext >= mmSlideTabSize )
	    slideNext = 0;
	if ( mmSlideTab[slideHead] > 0 ) {
	    if ( mmSlideTab[slideHead] - 1 > mmSlideTab[slideNext] )
		mmSlideTab[slideNext] = mmSlideTab[slideHead] - 1;
	    mmSlideTab[slideHead] = 0;
	} else {
	    if ( matchHere() )
		return TRUE;
	}

	if ( mmPos == lastPos )
	    break;

	/*
	  Update the slide table.  This code has much in common with the
	  initialization code.
	*/
	int sk = occ1[BadChar(mmIn[mmPos + minl])];
	if ( sk == NoOccurrence ) {
	    mmSlideTab[slideNext] = minl;
	} else if ( sk > 0 ) {
	    int k = slideNext + minl - sk;
	    if ( k >= mmSlideTabSize )
		k -= mmSlideTabSize;
	    if ( sk > mmSlideTab[k] )
		mmSlideTab[k] = sk;
	}
	slideHead = slideNext;
	mmPos++;
    }
    return FALSE;
}
#else
bool QRegExpEngine::bruteMatch()
{
    while ( mmPos <= mmLen ) {
	if ( matchHere() )
	    return TRUE;
	mmPos++;
    }
    return FALSE;
}
#endif

/*
  Here's the core of the engine.  It tries to do a match here and now.
*/
bool QRegExpEngine::matchHere()
{
    int ncur = 1, nnext = 0;
    int i = 0, j, k, m;
    bool match = FALSE;

    mmMatchedLen = -1;
    mmCurStack[0] = InitialState;

#ifndef QT_NO_REGEXP_CAPTURE
    if ( ncap > 0 ) {
	for ( j = 0; j < ncap; j++ ) {
	    mmCurCapBegin[j] = EmptyCapture;
	    mmCurCapEnd[j] = EmptyCapture;
	}
    }
#endif

#ifndef QT_NO_REGEXP_BACKREF
    int *zzZ = 0;

    while ( (ncur > 0 || mmSleeping.count() > 0) && i <= mmLen - mmPos &&
	    !match )
#else
    while ( ncur > 0 && i <= mmLen - mmPos && !match )
#endif
    {
	int ch = ( i < mmLen - mmPos ) ? mmIn[mmPos + i].unicode() : 0;
	for ( j = 0; j < ncur; j++ ) {
	    int cur = mmCurStack[j];
	    State *scur = s[cur];
	    QArray<int>& outs = scur->outs;
	    for ( k = 0; k < (int) outs.size(); k++ ) {
		int next = outs[k];
		State *snext = s[next];
		bool in = TRUE;
#ifndef QT_NO_REGEXP_BACKREF
		int needSomeSleep = 0;
#endif

		/*
		  First, check if the anchors are anchored properly.
		*/
		if ( scur->anchors != 0 ) {
		    int a = at( *scur->anchors, next );
		    if ( a != 0 && !testAnchor(i, a, mmCurCapBegin + j * ncap) )
			in = FALSE;
		}
		/*
		  If indeed they are, check if the input character is correct
		  for this transition.
		*/
		if ( in ) {
		    m = snext->match;
		    if ( (m & (CharClassBit | BackRefBit)) == 0 ) {
			if ( cs )
			    in = ( m == ch );
			else
			    in = ( QChar(m).lower() == QChar(ch).lower() );
		    } else if ( next == FinalState ) {
			mmMatchedLen = i;
			match = mmMinimal;
			in = TRUE;
		    } else if ( (m & CharClassBit) != 0 ) {
#ifndef QT_NO_REGEXP_CCLASS
			const CharClass *cc = cl[m ^ CharClassBit];
			if ( cs )
			    in = cc->in( ch );
			else if ( cc->negative() )
			    in = cc->in( QChar(ch).lower() ) &&
				 cc->in( QChar(ch).upper() );
			else
			    in = cc->in( QChar(ch).lower() ) ||
				 cc->in( QChar(ch).upper() );
#endif
#ifndef QT_NO_REGEXP_BACKREF
		    } else { /* ( (m & BackRefBit) != 0 ) */
			int bref = m ^ BackRefBit;
			int ell = j * ncap + ( bref - 1 );

			in = bref <= ncap && mmCurCapBegin[ell] != EmptyCapture;
			if ( in ) {
			    if ( cs )
				in = ( mmIn[mmPos + mmCurCapBegin[ell]]
				       == QChar(ch) );
			    else
				in = ( mmIn[mmPos + mmCurCapBegin[ell]].lower()
				       == QChar(ch).lower() );
			}

			if ( in ) {
			    int delta;
			    if ( mmCurCapEnd[ell] == EmptyCapture )
				delta = i - mmCurCapBegin[ell];
			    else
				delta = mmCurCapEnd[ell] - mmCurCapBegin[ell];

			    in = ( delta <= mmLen - mmPos );
			    if ( in && delta > 1 ) {
				int n;
				if ( cs ) {
				    for ( n = 1; n < delta; n++ ) {
					if ( mmIn[mmPos +
						  mmCurCapBegin[ell] + n] !=
					     mmIn[mmPos + i + n] )
					    break;
				    }
				} else {
				    for ( n = 1; n < delta; n++ ) {
					QChar a = mmIn[mmPos +
						       mmCurCapBegin[ell] + n];
					QChar b = mmIn[mmPos + i + n];
					if ( a.lower() != b.lower() )
					    break;
				    }
				}
				in = ( n == delta );
				if ( in )
				    needSomeSleep = delta - 1;
			    }
			}
#endif
		    }
		}

		/*
		  All is right.  We must now update our data structures.
		*/
		if ( in ) {
#ifndef QT_NO_REGEXP_CAPTURE
		    int *capBegin, *capEnd;
#endif
		    /*
		      If the next state was not encountered yet, all is fine.
		    */
		    if ( (m = mmInNextStack[next]) == -1 ) {
			m = nnext++;
			mmNextStack[m] = next;
			mmInNextStack[next] = m;
#ifndef QT_NO_REGEXP_CAPTURE
			capBegin = mmNextCapBegin + m * ncap;
			capEnd = mmNextCapEnd + m * ncap;

		    /*
		      Otherwise, we'll first maintain captures in temporary
		      arrays, and decide at the end whether it's best to keep
		      the previous capture zones or the new ones.
		    */
		    } else {
			capBegin = mmTempCapBegin;
			capEnd = mmTempCapEnd;
#endif
		    }

#ifndef QT_NO_REGEXP_CAPTURE
		    /*
		      Updating the capture zones is much of a task.
		    */
		    if ( ncap > 0 ) {
			memcpy( capBegin, mmCurCapBegin + j * ncap,
				ncap * sizeof(int) );
			memcpy( capEnd, mmCurCapEnd + j * ncap,
				ncap * sizeof(int) );
			int c = scur->atom, n = snext->atom;
			int p = -1, q = -1;
			int cap;

			/*
			  Lemma 1.  For any x in the range [0..nf), we have
			  f[x].parent < x.

			  Proof.  By looking at startAtom(), it is clear that
			  cf < nf holds all the time, and thus that
			  f[nf].parent < nf.
			*/

			/*
			  If we are reentering an atom, we empty all capture
			  zones inside it.
			*/
			if ( scur->reenter != 0 &&
			     (q = at(*scur->reenter, next)) != 0 ) {
			    QBitArray b;
			    b.fill( FALSE, nf );
			    b.setBit( q, TRUE );
			    for ( int ell = q + 1; ell < nf; ell++ ) {
				if ( b.testBit(f[ell].parent) ) {
				    b.setBit( ell, TRUE );
				    cap = f[ell].capture;
				    if ( cap >= 0 ) {
					capBegin[cap] = EmptyCapture;
					capEnd[cap] = EmptyCapture;
				    }
				}
			    }
			    p = f[q].parent;

			/*
			  Otherwise, close the capture zones we are leaving.
			  We are leaving f[c].capture, f[f[c].parent].capture,
			  f[f[f[c].parent].parent].capture, ..., until
			  f[x].capture, with x such that f[x].parent is the
			  youngest common ancestor for c and n.

			  We go up along c's and n's ancestry until we find x.
			*/
			} else {
			    p = c;
			    q = n;
			    while ( p != q ) {
				if ( p > q ) {
				    cap = f[p].capture;
				    if ( cap >= 0 ) {
					if ( capBegin[cap] == i ) {
					    capBegin[cap] = EmptyCapture;
					    capEnd[cap] = EmptyCapture;
					} else {
					    capEnd[cap] = i;
					}
				    }
				    p = f[p].parent;
				} else {
				    q = f[q].parent;
				}
			    }
			}

			/*
			  In any case, we now open the capture zones we are
			  entering.  We work upwards from n until we reach p
			  (the parent of the atom we reenter or the youngest
			  common ancestor).
			*/
			while ( n > p ) {
			    cap = f[n].capture;
			    if ( cap >= 0 ) {
				capBegin[cap] = i;
				capEnd[cap] = EmptyCapture;
			    }
			    n = f[n].parent;
			}
			/*
			  If the next state was already in mmNextStack, we must
			  choose carefully which capture zones we want to keep.
			*/
			if ( capBegin == mmTempCapBegin &&
			     isBetterCapture(capBegin, capEnd,
					     mmNextCapBegin + m * ncap,
					     mmNextCapEnd + m * ncap) ) {
			    memcpy( mmNextCapBegin + m * ncap, capBegin,
				    ncap * sizeof(int) );
			    memcpy( mmNextCapEnd + m * ncap, capEnd,
				    ncap * sizeof(int) );
			}
		    }
#ifndef QT_NO_REGEXP_BACKREF
		    /*
		      We are done with updating the capture zones.  It's now
		      time to put the next state to sleep, if it needs to, and
		      to remove it from mmNextStack.
		    */
		    if ( needSomeSleep > 0 ) {
			zzZ = new int[1 + 2 * ncap];
			zzZ[0] = next;
			if ( ncap > 0 ) {
			    memcpy( zzZ + 1, capBegin, ncap * sizeof(int) );
			    memcpy( zzZ + 1 + ncap, capEnd,
				    ncap * sizeof(int) );
			}
			mmInNextStack[mmNextStack[--nnext]] = -1;
			mmSleeping.insert( i + needSomeSleep, zzZ );
		    }
#endif
#endif
		}
	    }
	}
#ifndef QT_NO_REGEXP_CAPTURE
	/*
	  If we reached the final state, hurray!  Copy the captured zone.
	*/
	if ( ncap > 0 && (m = mmInNextStack[FinalState]) != -1 ) {
	    memcpy( mmCapBegin, mmNextCapBegin + m * ncap, ncap * sizeof(int) );
	    memcpy( mmCapEnd, mmNextCapEnd + m * ncap, ncap * sizeof(int) );
	}
#ifndef QT_NO_REGEXP_BACKREF
	/*
	  It's time to wake up the sleepers.
	*/
	if ( mmSleeping.count() > 0 ) {
	    while ( (zzZ = mmSleeping.take(i)) != 0 ) {
		int next = zzZ[0];
		int *capBegin = zzZ + 1;
		int *capEnd = zzZ + 1 + ncap;
		bool copyOver = TRUE;

		if ( (m = mmInNextStack[zzZ[0]]) == -1 ) {
		    m = nnext++;
		    mmNextStack[m] = next;
		    mmInNextStack[next] = m;
		} else {
		    copyOver = isBetterCapture( mmNextCapBegin + m * ncap,
						mmNextCapEnd + m * ncap,
						capBegin, capEnd );
		}
		if ( copyOver ) {
		    memcpy( mmNextCapBegin + m * ncap, capBegin,
			    ncap * sizeof(int) );
		    memcpy( mmNextCapEnd + m * ncap, capEnd,
			    ncap * sizeof(int) );
		}
		delete[] zzZ;
	    }
	}
#endif
#endif
	for ( j = 0; j < nnext; j++ )
	    mmInNextStack[mmNextStack[j]] = -1;

	qSwap( mmCurStack, mmNextStack );
#ifndef QT_NO_REGEXP_CAPTURE
	qSwap( mmCurCapBegin, mmNextCapBegin );
	qSwap( mmCurCapEnd, mmNextCapEnd );
#endif
	ncur = nnext;
	nnext = 0;
	i++;
    }

#ifndef QT_NO_REGEXP_BACKREF
    /*
      If minimal matching is enabled, we might have some sleepers left.
    */
    while ( !mmSleeping.isEmpty() ) {
	zzZ = mmSleeping.take( *QIntDictIterator<int>(mmSleeping) );
	delete[] zzZ;
    }
#endif

    match = ( mmMatchedLen >= 0 );
    if ( !match )
	mmMatchedLen = i - 1;
    return match;
}

#ifndef QT_NO_REGEXP_CCLASS

QRegExpEngine::CharClass::CharClass()
    : c( 0 ), n( FALSE )
#ifndef QT_NO_REGEXP_OPTIM
      , occ1( *noOccurrences )
#endif
{
}

QRegExpEngine::CharClass& QRegExpEngine::CharClass::operator=(
	const CharClass& cc )
{
    c = cc.c;
    r = cc.r.copy();
    n = cc.n;
#ifndef QT_NO_REGEXP_OPTIM
    occ1 = cc.occ1;
#endif
    return *this;
}

void QRegExpEngine::CharClass::clear()
{
    c = 0;
    r.resize( 0 );
    n = FALSE;
}

void QRegExpEngine::CharClass::setNegative( bool negative )
{
    n = negative;
#ifndef QT_NO_REGEXP_OPTIM
    occ1 = *firstOccurrenceAtZero;
#endif
}

void QRegExpEngine::CharClass::addCategories( int cats )
{
    c |= cats;
#ifndef QT_NO_REGEXP_OPTIM
    occ1 = *firstOccurrenceAtZero;
#endif
}

void QRegExpEngine::CharClass::addRange( ushort from, ushort to )
{
    if ( from > to )
	qSwap( from, to );
    int n = r.size();
    r.resize( n + 1 );
    r[n].from = from;
    r[n].to = to;

#ifndef QT_NO_REGEXP_OPTIM
    int i;

    if ( to - from < NumBadChars ) {
	occ1.detach();
	if ( from % NumBadChars <= to % NumBadChars ) {
	    for ( i = from % NumBadChars; i <= to % NumBadChars; i++ )
		occ1[i] = 0;
	} else {
	    for ( i = 0; i <= to % NumBadChars; i++ )
		occ1[i] = 0;
	    for ( i = from % NumBadChars; i < NumBadChars; i++ )
		occ1[i] = 0;
	}
    } else {
	occ1 = *firstOccurrenceAtZero;
    }
#endif
}

bool QRegExpEngine::CharClass::in( QChar ch ) const
{
#ifndef QT_NO_REGEXP_OPTIM
    if ( occ1[BadChar(ch)] == NoOccurrence )
	return n;
#endif

    if ( c != 0 && (c & (1 << (int) ch.category())) != 0 )
	return !n;
    for ( int i = 0; i < (int) r.size(); i++ ) {
	if ( ch.unicode() >= r[i].from && ch.unicode() <= r[i].to )
	    return !n;
    }
    return n;
}

#if defined(QT_DEBUG)
void QRegExpEngine::CharClass::dump() const
{
    int i;
    qDebug( "    %stive character class", n ? "nega" : "posi" );
#ifndef QT_NO_REGEXP_CCLASS
    if ( c != 0 )
	qDebug( "      categories 0x%.8x", c );
#endif
    for ( i = 0; i < (int) r.size(); i++ )
	qDebug( "      0x%.4x through 0x%.4x", r[i].from, r[i].to );
}
#endif
#endif

QRegExpEngine::Box::Box( QRegExpEngine *engine )
    : eng( engine ), skipanchors( 0 )
#ifndef QT_NO_REGEXP_OPTIM
      , earlyStart( 0 ), lateStart( 0 ), maxl( 0 ), occ1( *noOccurrences )
#endif
{
    minl = 0;
}

QRegExpEngine::Box& QRegExpEngine::Box::operator=( const Box& b )
{
    eng = b.eng;
    ls = b.ls;
    rs = b.rs;
    lanchors = b.lanchors;
    ranchors = b.ranchors;
    skipanchors = b.skipanchors;
#ifndef QT_NO_REGEXP_OPTIM
    earlyStart = b.earlyStart;
    lateStart = b.lateStart;
    str = b.str;
    leftStr = b.leftStr;
    rightStr = b.rightStr;
    maxl = b.maxl;
    occ1 = b.occ1;
#endif
    minl = b.minl;
    return *this;
}

void QRegExpEngine::Box::set( QChar ch )
{
    ls.resize( 1 );
    ls[0] = eng->createState( ch );
    rs = ls;
    rs.detach();
#ifndef QT_NO_REGEXP_OPTIM
    str = ch;
    leftStr = ch;
    rightStr = ch;
    maxl = 1;
    occ1.detach();
    occ1[BadChar(ch)] = 0;
#endif
    minl = 1;
}

void QRegExpEngine::Box::set( const CharClass& cc )
{
    ls.resize( 1 );
    ls[0] = eng->createState( cc );
    rs = ls;
    rs.detach();
#ifndef QT_NO_REGEXP_OPTIM
    maxl = 1;
    occ1 = cc.firstOccurrence();
#endif
    minl = 1;
}

#ifndef QT_NO_REGEXP_BACKREF
void QRegExpEngine::Box::set( int bref )
{
    ls.resize( 1 );
    ls[0] = eng->createState( bref );
    rs = ls;
    rs.detach();
    if ( bref >= 1 && bref <= MaxBackRefs )
	skipanchors = Anchor_BackRef0Empty << bref;
#ifndef QT_NO_REGEXP_OPTIM
    maxl = InftyLen;
#endif
    minl = 0;
}
#endif

void QRegExpEngine::Box::cat( const Box& b )
{
    eng->addCatTransitions( rs, b.ls );
    addAnchorsToEngine( b );
    if ( minl == 0 ) {
	mergeInto( &lanchors, b.lanchors );
	if ( skipanchors != 0 ) {
	    for ( int i = 0; i < (int) b.ls.size(); i++ ) {
		int a = eng->anchorConcatenation( at(lanchors, b.ls[i]),
						  skipanchors );
		lanchors.insert( b.ls[i], a );
	    }
	}
	mergeInto( &ls, b.ls );
    }
    if ( b.minl == 0 ) {
	mergeInto( &ranchors, b.ranchors );
	if ( b.skipanchors != 0 ) {
	    for ( int i = 0; i < (int) rs.size(); i++ ) {
		int a = eng->anchorConcatenation( at(ranchors, rs[i]),
						  b.skipanchors );
		ranchors.insert( rs[i], a );
	    }
	}
	mergeInto( &rs, b.rs );
    } else {
	ranchors = b.ranchors;
	rs = b.rs;
    }

#ifndef QT_NO_REGEXP_OPTIM
    if ( maxl != InftyLen ) {
	if ( rightStr.length() + b.leftStr.length() >
	     QMAX(str.length(), b.str.length()) ) {
	    earlyStart = minl - rightStr.length();
	    lateStart = maxl - rightStr.length();
	    str = rightStr + b.leftStr;
	} else if ( b.str.length() > str.length() ) {
	    earlyStart = minl + b.earlyStart;
	    lateStart = maxl + b.lateStart;
	    str = b.str;
	}
    }

    if ( (int) leftStr.length() == maxl )
	leftStr += b.leftStr;
    if ( (int) b.rightStr.length() == b.maxl )
	rightStr += b.rightStr;
    else
	rightStr = b.rightStr;

    if ( maxl == InftyLen || b.maxl == InftyLen )
	maxl = InftyLen;
    else
	maxl += b.maxl;

    occ1.detach();
    for ( int i = 0; i < NumBadChars; i++ ) {
	if ( b.occ1[i] != NoOccurrence && minl + b.occ1[i] < occ1[i] )
	    occ1[i] = minl + b.occ1[i];
    }
#endif

    minl += b.minl;
    if ( minl == 0 )
	skipanchors = eng->anchorConcatenation( skipanchors, b.skipanchors );
    else
	skipanchors = 0;
}

void QRegExpEngine::Box::orx( const Box& b )
{
    mergeInto( &ls, b.ls );
    mergeInto( &lanchors, b.lanchors );
    mergeInto( &rs, b.rs );
    mergeInto( &ranchors, b.ranchors );
    skipanchors = eng->anchorAlternation( skipanchors, b.skipanchors );

#ifndef QT_NO_REGEXP_OPTIM
    occ1.detach();
    for ( int i = 0; i < NumBadChars; i++ ) {
	if ( occ1[i] > b.occ1[i] )
	    occ1[i] = b.occ1[i];
    }
    earlyStart = 0;
    lateStart = 0;
    str = QString::null;
    leftStr = QString::null;
    rightStr = QString::null;
    if ( b.maxl > maxl )
	maxl = b.maxl;
#endif
    if ( b.minl < minl )
	minl = b.minl;
}

void QRegExpEngine::Box::plus( int atom )
{
#ifndef QT_NO_REGEXP_CAPTURE
    eng->addPlusTransitions( rs, ls, atom );
#else
    Q_UNUSED( atom );
    eng->addCatTransitions( rs, ls );
#endif
    addAnchorsToEngine( *this );
#ifndef QT_NO_REGEXP_OPTIM
    maxl = InftyLen;
#endif
}

void QRegExpEngine::Box::opt()
{
#ifndef QT_NO_REGEXP_OPTIM
    earlyStart = 0;
    lateStart = 0;
    str = QString::null;
    leftStr = QString::null;
    rightStr = QString::null;
#endif
    skipanchors = 0;
    minl = 0;
}

void QRegExpEngine::Box::catAnchor( int a )
{
    if ( a != 0 ) {
	for ( int i = 0; i < (int) rs.size(); i++ ) {
	    a = eng->anchorConcatenation( at(ranchors, rs[i]), a );
	    ranchors.insert( rs[i], a );
	}
	if ( minl == 0 )
	    skipanchors = eng->anchorConcatenation( skipanchors, a );
    }
}

#ifndef QT_NO_REGEXP_OPTIM
void QRegExpEngine::Box::setupHeuristics()
{
    eng->setupGoodStringHeuristic( earlyStart, lateStart, str );

    /*
      A regular expression such as 112|1 has occ1['2'] = 2 and minl = 1 at this
      point.  An entry of occ1 has to be at most minl or infinity for the rest
      of the algorithm to go well.

      We waited until here before normalizing these cases (instead of doing it
      in Box::orx()) because sometimes things improve by themselves; consider
      (112|1)34.
    */
    for ( int i = 0; i < NumBadChars; i++ ) {
	if ( occ1[i] != NoOccurrence && occ1[i] >= minl )
	    occ1[i] = minl;
    }
    eng->setupBadCharHeuristic( minl, occ1 );

    eng->heuristicallyChooseHeuristic();
}
#endif

#if defined(QT_DEBUG)
void QRegExpEngine::Box::dump() const
{
    int i;
    qDebug( "Box of at least %d character%s", minl, minl == 1 ? "" : "s" );
    qDebug( "  Left states:" );
    for ( i = 0; i < (int) ls.size(); i++ ) {
	if ( at(lanchors, ls[i]) == 0 )
	    qDebug( "    %d", ls[i] );
	else
	    qDebug( "    %d [anchors 0x%.8x]", ls[i], lanchors[ls[i]] );
    }
    qDebug( "  Right states:" );
    for ( i = 0; i < (int) rs.size(); i++ ) {
	if ( at(ranchors, ls[i]) == 0 )
	    qDebug( "    %d", rs[i] );
	else
	    qDebug( "    %d [anchors 0x%.8x]", rs[i], ranchors[rs[i]] );
    }
    qDebug( "  Skip anchors: 0x%.8x", skipanchors );
}
#endif

void QRegExpEngine::Box::addAnchorsToEngine( const Box& to ) const
{
    for ( int i = 0; i < (int) to.ls.size(); i++ ) {
	for ( int j = 0; j < (int) rs.size(); j++ ) {
	    int a = eng->anchorConcatenation( at(ranchors, rs[j]),
					      at(to.lanchors, to.ls[i]) );
	    eng->addAnchors( rs[j], to.ls[i], a );
	}
    }
}

int QRegExpEngine::getChar()
{
    return ( yyPos == yyLen ) ? EOS : yyIn[yyPos++].unicode();
}

int QRegExpEngine::getEscape()
{
#ifndef QT_NO_REGEXP_ESCAPE
    const char tab[] = "afnrtv"; // no b, as \b means word boundary
    const char backTab[] = "\a\f\n\r\t\v";
    ushort low;
    int i;
#endif
    ushort val;
    int prevCh = yyCh;

    if ( prevCh == EOS ) {
	yyError = TRUE;
	return Tok_Char | '\\';
    }
    yyCh = getChar();
#ifndef QT_NO_REGEXP_ESCAPE
    if ( (prevCh & ~0xff) == 0 ) {
	const char *p = strchr( tab, prevCh );
	if ( p != 0 )
	    return Tok_Char | backTab[p - tab];
    }
#endif

    switch ( prevCh ) {
#ifndef QT_NO_REGEXP_ESCAPE
    case '0':
	val = 0;
	for ( i = 0; i < 3; i++ ) {
	    if ( yyCh >= '0' && yyCh <= '7' )
		val = ( val << 3 ) | ( yyCh - '0' );
	    else
		break;
	    yyCh = getChar();
	}
	if ( (val & ~0377) != 0 )
	    yyError = TRUE;
	return Tok_Char | val;
#endif
#ifndef QT_NO_REGEXP_ESCAPE
    case 'B':
	return Tok_NonWord;
#endif
#ifndef QT_NO_REGEXP_CCLASS
    case 'D':
	// see QChar::isDigit()
	yyCharClass->addCategories( 0x7fffffef );
	return Tok_CharClass;
    case 'S':
	// see QChar::isSpace()
	yyCharClass->addCategories( 0x7ffff87f );
	yyCharClass->addRange( 0x0000, 0x0008 );
	yyCharClass->addRange( 0x000e, 0x001f );
	yyCharClass->addRange( 0x007f, 0x009f );
	return Tok_CharClass;
    case 'W':
	// see QChar::isLetterOrNumber()
	yyCharClass->addCategories( 0x7ff07f8f );
	return Tok_CharClass;
#endif
#ifndef QT_NO_REGEXP_ESCAPE
    case 'b':
	return Tok_Word;
#endif
#ifndef QT_NO_REGEXP_CCLASS
    case 'd':
	// see QChar::isDigit()
	yyCharClass->addCategories( 0x00000010 );
	return Tok_CharClass;
    case 's':
	// see QChar::isSpace()
	yyCharClass->addCategories( 0x00000380 );
	yyCharClass->addRange( 0x0009, 0x000d );
	return Tok_CharClass;
    case 'w':
	// see QChar::isLetterOrNumber()
	yyCharClass->addCategories( 0x000f8070 );
	return Tok_CharClass;
#endif
#ifndef QT_NO_REGEXP_ESCAPE
    case 'x':
	val = 0;
	for ( i = 0; i < 4; i++ ) {
	    low = QChar( yyCh ).lower();
	    if ( low >= '0' && low <= '9' )
		val = ( val << 4 ) | ( low - '0' );
	    else if ( low >= 'a' && low <= 'f' )
		val = ( val << 4 ) | ( low - 'a' + 10 );
	    else
		break;
	    yyCh = getChar();
	}
	return Tok_Char | val;
#endif
    default:
	if ( prevCh >= '1' && prevCh <= '9' ) {
#ifndef QT_NO_REGEXP_BACKREF
	    val = prevCh - '0';
	    while ( yyCh >= '0' && yyCh <= '9' ) {
		val = ( val *= 10 ) | ( yyCh - '0' );
		yyCh = getChar();
	    }
	    return Tok_BackRef | val;
#else
	    yyError = TRUE;
#endif
	}
	return Tok_Char | prevCh;
    }
}

#ifndef QT_NO_REGEXP_INTERVAL
int QRegExpEngine::getRep( int def )
{
    if ( yyCh >= '0' && yyCh <= '9' ) {
	int rep = 0;
	do {
	    rep = 10 * rep + yyCh - '0';
	    if ( rep >= InftyRep ) {
		yyError = TRUE;
		rep = def;
	    }
	    yyCh = getChar();
	} while ( yyCh >= '0' && yyCh <= '9' );
	return rep;
    } else {
	return def;
    }
}
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
void QRegExpEngine::skipChars( int n )
{
    if ( n > 0 ) {
	yyPos += n - 1;
	yyCh = getChar();
    }
}
#endif

void QRegExpEngine::startTokenizer( const QChar *rx, int len )
{
    yyIn = rx;
    yyPos0 = 0;
    yyPos = 0;
    yyLen = len;
    yyCh = getChar();
    yyCharClass = new CharClass;
    yyMinRep = 0;
    yyMaxRep = 0;
    yyError = FALSE;
}

int QRegExpEngine::getToken()
{
#ifndef QT_NO_REGEXP_CCLASS
    ushort pendingCh = 0;
    bool charPending;
    bool rangePending;
    int tok;
#endif
    int prevCh = yyCh;

    yyPos0 = yyPos - 1;
#ifndef QT_NO_REGEXP_CCLASS
    yyCharClass->clear();
#endif
    yyMinRep = 0;
    yyMaxRep = 0;
    yyCh = getChar();
    switch ( prevCh ) {
    case EOS:
	yyPos0 = yyPos;
	return Tok_Eos;
    case '$':
	return Tok_Dollar;
    case '(':
	if ( yyCh == '?' ) {
	    prevCh = getChar();
	    yyCh = getChar();
	    switch ( prevCh ) {
#ifndef QT_NO_REGEXP_LOOKAHEAD
	    case '!':
		return Tok_NegLookahead;
	    case '=':
		return Tok_PosLookahead;
#endif
	    case ':':
		return Tok_MagicLeftParen;
	    default:
		yyError = TRUE;
		return Tok_MagicLeftParen;
	    }
	} else {
	    return Tok_LeftParen;
	}
    case ')':
	return Tok_RightParen;
    case '*':
	yyMinRep = 0;
	yyMaxRep = InftyRep;
	return Tok_Quantifier;
    case '+':
	yyMinRep = 1;
	yyMaxRep = InftyRep;
	return Tok_Quantifier;
    case '.':
#ifndef QT_NO_REGEXP_CCLASS
	yyCharClass->setNegative( TRUE );
#endif
	return Tok_CharClass;
    case '?':
	yyMinRep = 0;
	yyMaxRep = 1;
	return Tok_Quantifier;
    case '[':
#ifndef QT_NO_REGEXP_CCLASS
	if ( yyCh == '^' ) {
	    yyCharClass->setNegative( TRUE );
	    yyCh = getChar();
	}
	charPending = FALSE;
	rangePending = FALSE;
	do {
	    if ( yyCh == '-' && charPending && !rangePending ) {
		rangePending = TRUE;
		yyCh = getChar();
	    } else {
		if ( charPending && !rangePending ) {
		    yyCharClass->addSingleton( pendingCh );
		    charPending = FALSE;
		}
		if ( yyCh == '\\' ) {
		    yyCh = getChar();
		    tok = getEscape();
		    if ( tok == Tok_Word )
			tok = '\b';
		} else {
		    tok = Tok_Char | yyCh;
		    yyCh = getChar();
		}
		if ( tok == Tok_CharClass ) {
		    if ( rangePending ) {
			yyCharClass->addSingleton( '-' );
			yyCharClass->addSingleton( pendingCh );
			charPending = FALSE;
			rangePending = FALSE;
		    }
		} else if ( (tok & Tok_Char) != 0 ) {
		    if ( rangePending ) {
			yyCharClass->addRange( pendingCh, tok ^ Tok_Char );
			charPending = FALSE;
			rangePending = FALSE;
		    } else {
			pendingCh = tok ^ Tok_Char;
			charPending = TRUE;
		    }
		} else {
		    yyError = TRUE;
		}
	    }
	}  while ( yyCh != ']' && yyCh != EOS );
	if ( rangePending )
	    yyCharClass->addSingleton( '-' );
	if ( charPending )
	    yyCharClass->addSingleton( pendingCh );
	if ( yyCh == EOS )
	    yyError = TRUE;
	else
	    yyCh = getChar();
	return Tok_CharClass;
#else
	yyError = TRUE;
	return Tok_Char | '[';
#endif
    case '\\':
	return getEscape();
    case ']':
	yyError = TRUE;
	return Tok_Char | ']';
    case '^':
	return Tok_Caret;
#ifndef QT_NO_REGEXP_INTERVAL
    case '{':
	yyMinRep = getRep( 0 );
	yyMaxRep = yyMinRep;
	if ( yyCh == ',' ) {
	    yyCh = getChar();
	    yyMaxRep = getRep( InftyRep );
	}
	if ( yyMaxRep < yyMinRep )
	    qSwap( yyMinRep, yyMaxRep );
	if ( yyCh != '}' )
	    yyError = TRUE;
	yyCh = getChar();
	return Tok_Quantifier;
#else
	yyError = TRUE;
	return Tok_Char | '{';
#endif
    case '|':
	return Tok_Bar;
    case '}':
	yyError = TRUE;
	return Tok_Char | '}';
    default:
	return Tok_Char | prevCh;
    }
}

int QRegExpEngine::parse( const QChar *pattern, int len )
{
    valid = TRUE;
    startTokenizer( pattern, len );
    yyTok = getToken();
#ifndef QT_NO_REGEXP_CAPTURE
    yyMayCapture = TRUE;
#else
    yyMayCapture = FALSE;
#endif

#ifndef QT_NO_REGEXP_CAPTURE
    int atom = startAtom( FALSE );
#endif
    CharClass anything;
    Box box( this ); // create InitialState
    box.set( anything );
    Box rightBox( this ); // create FinalState
    rightBox.set( anything );

    Box middleBox( this );
    parseExpression( &middleBox );
#ifndef QT_NO_REGEXP_CAPTURE
    finishAtom( atom );
#endif
#ifndef QT_NO_REGEXP_OPTIM
    middleBox.setupHeuristics();
#endif
    box.cat( middleBox );
    box.cat( rightBox );
    delete yyCharClass;
    yyCharClass = 0;

    realncap = ncap;
#ifndef QT_NO_REGEXP_BACKREF
    if ( nbrefs > ncap )
	ncap = nbrefs;
#endif

    mmCaptured.resize( 2 + 2 * realncap );
    mmCapturedNoMatch.fill( -1, 2 + 2 * realncap );

    /*
      We use one QArray<int> for all the big data used a lot in matchHere() and
      friends.
    */
#ifndef QT_NO_REGEXP_OPTIM
    mmSlideTabSize = QMAX( minl + 1, 16 );
#else
    mmSlideTabSize = 0;
#endif
    mmBigArray.resize( (3 + 4 * ncap) * ns + 4 * ncap + mmSlideTabSize );

    mmInNextStack = mmBigArray.data();
    memset( mmInNextStack, -1, ns * sizeof(int) );
    mmCurStack = mmInNextStack + ns;
    mmNextStack = mmInNextStack + 2 * ns;

    mmCurCapBegin = mmInNextStack + 3 * ns;
    mmNextCapBegin = mmCurCapBegin + ncap * ns;
    mmCurCapEnd = mmCurCapBegin + 2 * ncap * ns;
    mmNextCapEnd = mmCurCapBegin + 3 * ncap * ns;

    mmTempCapBegin = mmCurCapBegin + 4 * ncap * ns;
    mmTempCapEnd = mmTempCapBegin + ncap;
    mmCapBegin = mmTempCapBegin + 2 * ncap;
    mmCapEnd = mmTempCapBegin + 3 * ncap;

    mmSlideTab = mmTempCapBegin + 4 * ncap;

    if ( yyError )
	return -1;

#ifndef QT_NO_REGEXP_OPTIM
    State *sinit = s[InitialState];
    caretAnchored = ( sinit->anchors != 0 );
    if ( caretAnchored ) {
	QMap<int, int>& anchors = *sinit->anchors;
	QMap<int, int>::ConstIterator a;
	for ( a = anchors.begin(); a != anchors.end(); ++a ) {
#ifndef QT_NO_REGEXP_ANCHOR_ALT
	    if ( (*a & Anchor_Alternation) != 0 )
		break;
#endif
	    if ( (*a & Anchor_Caret) == 0 ) {
		caretAnchored = FALSE;
		break;
	    }
	}
    }
#endif
    return yyPos0;
}

void QRegExpEngine::parseAtom( Box *box )
{
#ifndef QT_NO_REGEXP_LOOKAHEAD
    QRegExpEngine *eng = 0;
    bool neg;
    int len;
#endif

    switch ( yyTok ) {
    case Tok_Dollar:
	box->catAnchor( Anchor_Dollar );
	break;
    case Tok_Caret:
	box->catAnchor( Anchor_Caret );
	break;
#ifndef QT_NO_REGEXP_LOOKAHEAD
    case Tok_PosLookahead:
    case Tok_NegLookahead:
	neg = ( yyTok == Tok_NegLookahead );
	eng = new QRegExpEngine( cs );
	len = eng->parse( yyIn + yyPos - 1, yyLen - yyPos + 1 );
	if ( len >= 0 )
	    skipChars( len );
	else
	    yyError = TRUE;
	box->catAnchor( addLookahead(eng, neg) );
	yyTok = getToken();
	if ( yyTok != Tok_RightParen )
	    yyError = TRUE;
	break;
#endif
#ifndef QT_NO_REGEXP_ESCAPE
    case Tok_Word:
	box->catAnchor( Anchor_Word );
	break;
    case Tok_NonWord:
	box->catAnchor( Anchor_NonWord );
	break;
#endif
    case Tok_LeftParen:
    case Tok_MagicLeftParen:
	yyTok = getToken();
	parseExpression( box );
	if ( yyTok != Tok_RightParen )
	    yyError = TRUE;
	break;
    case Tok_CharClass:
	box->set( *yyCharClass );
	break;
    default:
	if ( (yyTok & Tok_Char) != 0 )
	    box->set( QChar(yyTok ^ Tok_Char) );
#ifndef QT_NO_REGEXP_BACKREF
	else if ( (yyTok & Tok_BackRef) != 0 )
	    box->set( yyTok ^ Tok_BackRef );
#endif
	else
	    yyError = TRUE;
    }
    yyTok = getToken();
}

void QRegExpEngine::parseFactor( Box *box )
{
#ifndef QT_NO_REGEXP_CAPTURE
    int atom = startAtom( yyMayCapture && yyTok == Tok_LeftParen );
#else
    static const int atom = 0;
#endif

#ifndef QT_NO_REGEXP_INTERVAL
#define YYREDO() \
	yyIn = in, yyPos0 = pos0, yyPos = pos, yyLen = len, yyCh = ch, \
	*yyCharClass = charClass, yyMinRep = 0, yyMaxRep = 0, yyTok = tok

    const QChar *in = yyIn;
    int pos0 = yyPos0;
    int pos = yyPos;
    int len = yyLen;
    int ch = yyCh;
    CharClass charClass;
    if ( yyTok == Tok_CharClass )
	charClass = *yyCharClass;
    int tok = yyTok;
    bool mayCapture = yyMayCapture;
#endif

    parseAtom( box );
#ifndef QT_NO_REGEXP_CAPTURE
    finishAtom( atom );
#endif

    if ( yyTok == Tok_Quantifier ) {
	if ( yyMaxRep == InftyRep ) {
	    box->plus( atom );
#ifndef QT_NO_REGEXP_INTERVAL
	} else if ( yyMaxRep == 0 ) {
	    box->clear();
#endif
	}
	if ( yyMinRep == 0 )
	    box->opt();

#ifndef QT_NO_REGEXP_INTERVAL
	yyMayCapture = FALSE;
	int alpha = ( yyMinRep == 0 ) ? 0 : yyMinRep - 1;
	int beta = ( yyMaxRep == InftyRep ) ? 0 : yyMaxRep - ( alpha + 1 );

	Box rightBox( this );
	int i;

	for ( i = 0; i < beta; i++ ) {
	    YYREDO();
	    Box leftBox( this );
	    parseAtom( &leftBox );
	    leftBox.cat( rightBox );
	    leftBox.opt();
	    rightBox = leftBox;
	}
	for ( i = 0; i < alpha; i++ ) {
	    YYREDO();
	    Box leftBox( this );
	    parseAtom( &leftBox );
	    leftBox.cat( rightBox );
	    rightBox = leftBox;
	}
	rightBox.cat( *box );
	*box = rightBox;
#endif
	yyTok = getToken();
#ifndef QT_NO_REGEXP_INTERVAL
	yyMayCapture = mayCapture;
#endif
    }
#undef YYREDO
}

void QRegExpEngine::parseTerm( Box *box )
{
#ifndef QT_NO_REGEXP_OPTIM
    if ( yyTok != Tok_Eos && yyTok != Tok_RightParen && yyTok != Tok_Bar )
	parseFactor( box );
#endif
    while ( yyTok != Tok_Eos && yyTok != Tok_RightParen && yyTok != Tok_Bar ) {
	Box rightBox( this );
	parseFactor( &rightBox );
	box->cat( rightBox );
    }
}

void QRegExpEngine::parseExpression( Box *box )
{
    parseTerm( box );
    while ( yyTok == Tok_Bar ) {
	Box rightBox( this );
	yyTok = getToken();
	parseTerm( &rightBox );
	box->orx( rightBox );
    }
}

/*
  The class QRegExpPrivate contains the private data of a regular expression
  other than the automaton.  It makes it possible for many QRegExp objects to
  use the same QRegExpEngine object with different QRegExpPrivate objects.
*/
struct QRegExpPrivate
{
    QString pattern; // regular-expression or wildcard pattern
    QString rxpattern; // regular-expression pattern
#ifndef QT_NO_REGEXP_WILDCARD
    bool wc; // wildcard mode?
#endif
    bool min; // minimal matching? (instead of maximal)
#ifndef QT_NO_REGEXP_CAPTURE
    QString t; // last string passed to QRegExp::search() or searchRev()
    QStringList capturedCache; // what QRegExp::capturedTexts() returned last
#endif
    QArray<int> captured; // what QRegExpEngine::search() returned last

    QRegExpPrivate() { captured.fill( -1, 2 ); }
};

#ifndef QT_NO_REGEXP_OPTIM
static QCache<QRegExpEngine> *engineCache = 0;
#endif

static QRegExpEngine *newEngine( const QString& pattern, bool caseSensitive )
{
#ifndef QT_NO_REGEXP_OPTIM
    if ( engineCache != 0 ) {
	QRegExpEngine *eng = engineCache->take( pattern );
	if ( eng == 0 || eng->caseSensitive() != caseSensitive ) {
	    delete eng;
	} else {
	    eng->ref();
	    return eng;
	}
    }
#endif
    return new QRegExpEngine( pattern, caseSensitive );
}

static void derefEngine( QRegExpEngine *eng, const QString& pattern )
{
    if ( eng != 0 && eng->deref() ) {
#ifndef QT_NO_REGEXP_OPTIM
	if ( engineCache == 0 ) {
	    engineCache = new QCache<QRegExpEngine>;
	    engineCache->setAutoDelete( TRUE );
	}
	if ( !pattern.isNull() &&
	     engineCache->insert(pattern, eng, 4 + pattern.length() / 4) )
	    return;
#else
	Q_UNUSED( pattern );
#endif
	delete eng;
    }
}

/*!
  Constructs an empty expression.

  \sa isValid()
*/
QRegExp::QRegExp()
{
    eng = new QRegExpEngine( TRUE );
    priv = new QRegExpPrivate;
    priv->pattern = QString::null;
#ifndef QT_NO_REGEXP_WILDCARD
    priv->wc = FALSE;
#endif
    priv->min = FALSE;
    compile( TRUE );
}

/*!
  Constructs a regular expression object for a given \a pattern string.  The
  pattern may be given according to wildcard notation, if \a wildcard is TRUE;
  by default, the \a pattern is treated as a regular expression.  The pattern
  is case sensitive, unless \a caseSensitive is false.  Matching is maximal.

  \sa setPattern() setCaseSensitive() setWildcard() setMinimal()
*/
QRegExp::QRegExp( const QString& pattern, bool caseSensitive, bool wildcard )
{
    eng = 0;
    priv = new QRegExpPrivate;
    priv->pattern = pattern;
#ifndef QT_NO_REGEXP_WILDCARD
    priv->wc = wildcard;
#endif
    priv->min = FALSE;
    compile( caseSensitive );
}

/*!
  Constructs a regular expression as a copy of \a rx.

  \sa operator=()
*/
QRegExp::QRegExp( const QRegExp& rx )
{
    eng = 0;
    priv = new QRegExpPrivate;
    operator=( rx );
}

/*!
  Destructs the regular expression and cleans up its internal data.
*/
QRegExp::~QRegExp()
{
    derefEngine( eng, priv->rxpattern );
    delete priv;
}

/*!
  Copies the regular expression \a rx and returns a reference to this QRegExp.
  The case sensitivity, wildcard and minimal matching options are copied as
  well.
*/
QRegExp& QRegExp::operator=( const QRegExp& rx )
{
    rx.eng->ref();
    derefEngine( eng, priv->rxpattern );
    eng = rx.eng;
    priv->pattern = rx.priv->pattern;
    priv->rxpattern = rx.priv->rxpattern;
#ifndef QT_NO_REGEXP_WILDCARD
    priv->wc = rx.priv->wc;
#endif
    priv->min = rx.priv->min;
#ifndef QT_NO_REGEXP_CAPTURE
    priv->t = rx.priv->t;
    priv->capturedCache = rx.priv->capturedCache;
#endif
    priv->captured = rx.priv->captured;
    return *this;
}

/*!
  Returns TRUE if this regular expression is equal to \a rx, otherwise FALSE.

  Two QRegExp objects are equal if they have equal pattern strings, and if case
  sensitivity, wildcard and minimal matching options are identical.
*/
bool QRegExp::operator==( const QRegExp& rx ) const
{
    return priv->pattern == rx.priv->pattern &&
	   eng->caseSensitive() == rx.eng->caseSensitive() &&
#ifndef QT_NO_REGEXP_WILDCARD
	   priv->wc == rx.priv->wc &&
#endif
	   priv->min == rx.priv->min;
}

/*!  \fn bool QRegExp::operator!=( const QRegExp& rx ) const

  Returns TRUE if this regular expression is not equal to \a rx, otherwise
  FALSE.

  \sa operator==()
*/

/*!
  Returns TRUE if the pattern string is empty, otherwise FALSE.  An empty
  pattern matches an empty string.

  This is the same as pattern().\link QString::isEmpty() isEmpty() \endlink.
*/

bool QRegExp::isEmpty() const
{
    return priv->pattern.isEmpty();
}

/*!
  Returns TRUE if the regular expression is valid, or FALSE if it's invalid.  An
  invalid regular expression never matches.

  The pattern <b>[a-z</b> is an example of an invalid pattern, since it lacks
  a closing bracket.
*/
bool QRegExp::isValid() const
{
    return eng->isValid();
}

/*!
  Returns the pattern string of the regular expression.  The pattern has either
  regular expression syntax or wildcard syntax, depending on wildcard().

  \sa setPattern()
*/
QString QRegExp::pattern() const
{
    return priv->pattern;
}

/*!
  Sets the pattern string to \a pattern and returns a reference to this regular
  expression.  The case sensitivity, wildcard and minimal matching options are
  left alone.

  \sa pattern()
*/
void QRegExp::setPattern( const QString& pattern )
{
    if ( priv->pattern != pattern ) {
	priv->pattern = pattern;
	compile( caseSensitive() );
    }
}

/*!
  Returns TRUE if case sensitivity is enabled, otherwise FALSE.  The default is
  TRUE.

  \sa setCaseSensitive()
*/
bool QRegExp::caseSensitive() const
{
    return eng->caseSensitive();
}

/*!
  Sets case sensitive matching to \a sensitive.

  If case sensitivity is on, <b>T.X</b> matches <tt>TeX</tt> but not
  <tt>Tex</tt>.

  \sa caseSensitive()
*/
void QRegExp::setCaseSensitive( bool sensitive )
{
    if ( sensitive != eng->caseSensitive() )
	compile( sensitive );
}

#ifndef QT_NO_REGEXP_WILDCARD
/*!
  Returns TRUE if wildcard mode is enabled, otherwise FALSE.  The default is
  FALSE.

  \sa setWildcard()
*/
bool QRegExp::wildcard() const
{
    return priv->wc;
}

/*!  Sets the wildcard option for the regular expression.  The default is FALSE.

  Setting \a wildcard to TRUE makes it convenient to match filenames.

  For example, <b>l*.tex</b> matches the string <tt>labels.tex</tt> in wildcard
  mode, but not <tt>latex</tt>.

  \sa wildcard()
*/
void QRegExp::setWildcard( bool wildcard )
{
    if ( wildcard != priv->wc ) {
	priv->wc = wildcard;
	compile( caseSensitive() );
    }
}
#endif

/*!  Returns TRUE if minimal matching is enabled, otherwise FALSE.

  \sa setMinimal()
*/
bool QRegExp::minimal() const
{
    return priv->min;
}

/*!
  Enables or disables minimal matching.  If \a minimal is FALSE, matching is
  maximal (the default).

  For regular expression <b>\<.*\></b> and input string <tt>a\<b\>c\<d\>e</tt>,
  the minimal match is <tt>\<b\></tt> and the maximal match is
  <tt>\<b\>c\<d\></tt>.

  \sa minimal()
*/
void QRegExp::setMinimal( bool minimal )
{
    priv->min = minimal;
}

/*!
  Returns TRUE if \a str is matched exactly by this regular expression;
  otherwise, it returns FALSE and you can know how much of the string was
  matched correctly by calling matchedLength().

  For example, if the regular expression is <b>abc</b>, then this function
  returns TRUE only for input <tt>abc</tt>.  For inputs <tt>abcd</tt>,
  <tt>ab</tt> and <tt>hab</tt>, matchedLength() gives respectively 3, 2 and 0.

  \sa search() searchRev() QRegExpValidator
*/
bool QRegExp::match( const QString& str )
{
#ifndef QT_NO_REGEXP_CAPTURE
    priv->t = str;
    priv->capturedCache.clear();
#endif

    priv->captured = eng->match( str, 0, priv->min, TRUE );
    if ( priv->captured[1] == (int) str.length() ) {
	return TRUE;
    } else {
	priv->captured.detach();
	priv->captured[0] = 0;
	priv->captured[1] = eng->matchedLength();
	return FALSE;
    }
}

/*! \overload

  This version does not set matchedLength(), capturedTexts() and friends.
*/
bool QRegExp::match( const QString& str ) const
{
   return ( eng->match(str, 0, priv->min, TRUE)[0] == 0 );
}

/*! \obsolete

  Attempts to match in \a str, starting from position \a index.  Returns the
  position of the match, or -1 if there was no match.

  The length of the match is stored in \a *len, unless \a len is a null pointer.

  If \a indexIsStart is TRUE (the default), the position \a index in the string
  will match the start-of-input primitive (^) in the regexp, if present.
  Otherwise, position 0 in \a str will match.

  It's a good idea to use search() and matchedLength() instead of this function.
  If you really need the \a indexIsStart functionality, try this:

  \code
    QRegExp rx( "some text" );
    int pos = rx.search( str.mid(index) );
    if ( pos != -1 )
	pos += index;
    int len = rx.matchedLength();
  \endcode
*/
int QRegExp::match( const QString& str, int index, int *len,
		    bool indexIsStart )
{
    int pos;
    if ( indexIsStart ) {
	pos = search( str.mid(index) );
	if ( pos >= 0 ) {
	    pos += index;
	    if ( len != 0 )
		*len = matchedLength();
	} else {
	    if ( len != 0 )
		*len = 0;
	}
    } else {
	pos = search( str, index );
	if ( len != 0 )
	    *len = matchedLength();
    }
    return pos;
}

/*!
  Attempts to find a match in \a str from position \a start (0 by default).  If
  \a start is -1, the search starts at the last character; if -2, at the next to
  last character; etc.

  Returns the position of the first match, or -1 if there was no match.

  You might prefer to use QString::find(), QString::contains() or even
  QStringList::grep().

  Example:
  \code
    QRegExp rx( "[0-9]*\\.[0-9]+" );    // matches floating point
    int pos = rx.search( "pi = 3.14" ); // pos == 5
    int len = rx.matchedLength();       // len == 4
  \endcode

  \sa searchRev() match() matchedLength() capturedTexts()
*/
int QRegExp::search( const QString& str, int start )
{
    if ( start < 0 )
	start += str.length();
#ifndef QT_NO_REGEXP_CAPTURE
    priv->t = str;
    priv->capturedCache.clear();
#endif
    priv->captured = eng->match( str, start, priv->min, FALSE );
    return priv->captured[0];
}

/*! \overload

  This version does not set matchedLength(), capturedTexts() and friends.
*/
int QRegExp::search( const QString& str, int start ) const
{
    if ( start < 0 )
	start += str.length();
    return eng->match( str, start, priv->min, FALSE )[0];
}

/*!
  Attempts to find a match backwards in \a str from position \a start.  If
  \a start is -1 (the default), the search starts at the last character; if -2,
  at the next to last character; etc.

  Returns the position of the first match, or -1 if there was no match.

  You might prefer to use QString::findRev().

  \sa search() matchedLength() capturedTexts()
*/
int QRegExp::searchRev( const QString& str, int start )
{
    if ( start < 0 )
	start += str.length();
#ifndef QT_NO_REGEXP_CAPTURE
    priv->t = str;
    priv->capturedCache.clear();
#endif
    if ( start < 0 || start > (int) str.length() ) {
	priv->captured.detach();
	priv->captured.fill( -1 );
	return -1;
    }

    while ( start >= 0 ) {
	priv->captured = eng->match( str, start, priv->min, TRUE );
	if ( priv->captured[0] == start )
	    return start;
	start--;
    }
    return -1;
}

/*! \overload

  This version does not set matchedLength(), capturedText() and friends.
*/
int QRegExp::searchRev( const QString& str, int start ) const
{
    if ( start < 0 )
	start += str.length();
    if ( start < 0 || start > (int) str.length() )
	return -1;

    while ( start >= 0 ) {
	if ( eng->match(str, start, priv->min, TRUE)[0] == start )
	    return start;
	start--;
    }
    return -1;
}

/*!
  Returns the length of the last matched string, or -1 if there was no match.

  \sa match() search()
*/
int QRegExp::matchedLength()
{
    return priv->captured[1];
}

#ifndef QT_NO_REGEXP_CAPTURE
/*!
  Returns a list of the captured text strings.

  The returned list contains the entire matched string, followed by
  the strings matched by each subexpression. For example:

  \code
    QRegExp length( "(\d+)(cm|inch(es)?)" );
    int pos = length.search( "only 42cm long" );
    QStringList l = length.capturedTexts();
    // l is now ( "42cm", "42", "cm" )
  \endcode

  There is also a cap( n ) that returns the same as
  capturedTexts()[n], and a pos( n ) that returns the position where
  each match starts.

  The string list is ordered by the order of the '(' characters in the
  regular expression, as shown in the cap() documentation.

  \sa cap() pos()
*/
QStringList QRegExp::capturedTexts()
{
    if ( priv->capturedCache.isEmpty() ) {
	for ( int i = 0; i < (int) priv->captured.size(); i += 2 ) {
	    QString m;
	    if ( priv->captured[i + 1] == 0 )
		m = QString::fromLatin1( "" );
	    else if ( priv->captured[i] >= 0 )
		m = priv->t.mid( priv->captured[i],
				 priv->captured[i + 1] );
	    priv->capturedCache.append( m );
	}
	priv->t = QString::null;
    }
    return priv->capturedCache;
}

/*! Returns the text captured by the \a nth subexpression. The regular
  expression itself has index 0 and the parenthesised subexpression
  have indices 1 and above.

  \code
    QRegExp length( "(\d+)(cm|inch)" );
    int pos = length.search( "Not John Holmes: 14cm" );
    if ( pos > -1 ) {
	QString number = length.cap( 1 ); // "14"
        QString unit = length.cap( 2 ); // "cm"
	...
    }
  \endcode

  Note that if the subexpression is used several times, the last match
  is the one QRegExp remembers:

  \code
    QRegExp r( "results: ((\d+)(,\s)?)*" );
    int pos = length.search( "Here are the results: 14, 15, 12, 13" );
    if ( pos > -1 ) {
        QString measured = r.cap( 2 ); // "13"
	...
    }
  \endcode

  The last example is a bit hard, so we'll explain it more thoroughly.
  The core of the regexp is <tt>(\d+)</tt>, which matches one
  number. Around that, <tt>(\d+)(,\s)?</tt> matches a number
  optionally followed by a comma and a space. Finally
  <tt>((\d+)(,\s)?)*</tt> is a series of such numbers.

  cap() indexing follows the position of the '(' character. In this
  expression, the first parenthesis wraps <tt>(\d+)(,\s)?</tt>, so
  cap(1) returns the last number and its optional ", " suffix. The
  second parenthesis wraps <tt>(\d+)</tt>, so cap(2) returns the last
  number.

  There is also a function, pos(), that returns the position of each
  match, and one to return all of the subexpression matches,
  capturedTexts().

  \sa search()
*/
QString QRegExp::cap( int nth )
{
    if ( nth < 0 || nth >= (int) priv->captured.size() / 2 )
	return QString::null;
    else
	return capturedTexts()[nth];
}

/*! Returns the position of the \a nth captured text in the searched
  string.  If \a nth is 0 (the default), pos() returns the position of
  the whole match.

  Example:
  \code
    QRegExp rx( "/([a-z]+)/([a-z]+)" );
    rx.search( "to /dev/null now" );    // 3 (position of /dev/null)
    rx.pos( 0 );                        // 3 (position of /dev/null)
    rx.pos( 1 );                        // 4 (position of dev)
    rx.pos( 2 );                        // 8 (position of null)
  \endcode

  Note that pos() returns -1 for zero-length matches. (For example, if
  cap(4) would return an empty string, pos(4) returns -1.) This is due
  to an implementation tradeoff.

  \sa capturedTexts() cap()
*/
int QRegExp::pos( int nth )
{
    if ( nth < 0 || nth >= (int) priv->captured.size() / 2 )
	return -1;
    else
	return priv->captured[2 * nth];
}
#endif

void QRegExp::compile( bool caseSensitive )
{
    derefEngine( eng, priv->rxpattern );
#ifndef QT_NO_REGEXP_WILDCARD
    if ( priv->wc )
	priv->rxpattern = wc2rx( priv->pattern );
    else
#endif
	priv->rxpattern = priv->pattern.isNull() ? QString::fromLatin1( "" )
			  : priv->pattern;
    eng = newEngine( priv->rxpattern, caseSensitive );
#ifndef QT_NO_REGEXP_CAPTURE
    priv->t = QString::null;
    priv->capturedCache.clear();
#endif
    priv->captured.detach();
    priv->captured.fill( -1, 2 + 2 * eng->numCaptures() );
}
