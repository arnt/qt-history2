/*
  qregexp.cpp
*/

#include "qarray.h"
#include "qbitarray.h"
#include "qintdict.h"
#include "qmap.h"
#include "qregexp.h"
#include "qstring.h"
#include "qtl.h"
#include "qvector.h"

#include <limits.h>

/*
  WARNING!  Be sure to read qregexp.tex before modifying this file.  And drop an
  email to Jasmin Blanchette (jasmin@trolltech.com).
*/

/*!
  \class QRegExp qregexp.h

  \brief The QRegExp class provides pattern matching using regular expression
  and wildcard patterns.

  \ingroup tools
  \ingroup misc
  \ingroup shared

  In normal regular expression mode, you can use any of these atoms to match a
  single character:

  <ul plain>
  <li><b><em>c</em></b> matches the normal character <tt><em>c</em></tt>
  <li><b>\\e c </b> matches the special character <tt><em>c</em></tt>, one
  of<tt> $ ( ) * + - . ? [ \ ] ^ { | }</tt>
  <li><b>\a</b> matches the ASCII bell character (BEL, 0x07)
  <li><b>\f</b> matches the ASCII form feed character (FF, 0x0C)
  <li><b>\n</b> matches the ASCII line feed character (LF, 0x0A), also known as
  newline
  <li><b>\r</b> matches the ASCII carriage return character (CR, 0x0D)
  <li><b>\t</b> matches the ASCII horizontal tabulation character (HT, 0x09)
  <li><b>\v</b> matches the ASCII vertical tabulation character (VT, 0x0B)
  <li><b>\x\e hhhh </b> matches the Unicode character corresponding to the
  hexadecimal number \e hhhh (between 0x0000 and 0xFFFF)
  <li><b>\0\e ooo </b> (i.e., <b>\ zero <em>ooo</em></b>) matches the
  ASCII/Latin-1 character corresponding to the octal number \e ooo (between 0
  and 0377)
  <li><b>[</b>...<b>]</b> matches one of the characters in the specified set
  (see below)
  <li><b>.</b> matches any character (including newline)
  <li><b>\d</b> matches a digit (any character for which QChar::isDigit() is
  true)
  <li><b>\D</b> matches a non-digit
  <li><b>\s</b> matches a space (any character for which QChar::isSpace() is
  true)
  <li><b>\S</b> matches a non-space
  <li><b>\w</b> matches a word character (any character for which
  QChar::isLetterOrNumber() is true)
  <li><b>\W</b> matches a non-word character.
  </ul>

  The following atoms make an assertion without consuming any input:

  <ul plain>
  <li><b>\b</b> matches at a word boundary
  <li><b>\B</b> matches wherever <b>\b</b> doesn't match
  <li><b>^</b> matches at the beginning of input
  <li><b>$</b> matches at the end of input
  <li><b>(?=<em>E</em>)</b> matches if the next characters of input match the
  regular expression <b><em>E</em></b> (positive lookahead)
  <li><b>(?!<em>E</em>)</b> matches if the next characters of input do not
  match the regular expression <b><em>E</em></b> (negative lookahead).
  </ul>

  You may use one of the following quantifiers to match a certain number of
  occurrences of an atom <b><em>A</em></b> (at most 999):

  <ul plain>
  <li><b><em>A</em>*</b> matches 0 or more occurrences of <b><em>A</em></b>
  <li><b><em>A</em>?</b> matches 0 or 1 occurrences of <b><em>A</em></b>
  <li><b><em>A</em>+</b> matches 1 or more occurrences of <b><em>A</em></b>
  <li><b><em>A</em>{<em>m</em>}</b> matches exactly \e m occurrences of
  <b><em>A</em></b>
  <li><b><em>A</em>{<em>m</em>,<em>n</em>}</b> matches between \e m and \e n
  occurrences of <b><em>A</em></b>
  <li><b><em>A</em>{<em>m</em>,}</b> matches at least \e m occurrences of
  <b><em>A</em></b>
  <li><b><em>A</em>{,<em>n</em>}</b> matches at most \e n occurrences of
  <b><em>A</em></b>.
  </ul>

  You can always build an atom by enclosing a regular expression
  <b><em>E</em></b> in parentheses:

  <ul plain>
  <li><b>(<em>E</em>)</b> matches the sub-expression <b><em>E</em></b> and
  captures the matched text, making it available through back-references and
  capturedText()
  <li><b>(?:<em>E</em>)</b> matches the sub-expression <b><em>E</em></b>
  without capturing anything.
  </ul>

  Back-references are expressions of the form <b>&#92;<em>N</em></b> that stand
  for the <em>N</em>th substring matched by the <em>N</em>th parenthesized
  sub-expression.  For example, <b>(.)(.)\2\1</b> matches all palindromes of
  length 4 (e.g., <tt>anna</tt>), and <b>(a\1*)</b> a power of 2 of
  <tt>a</tt>'s (e.g., <tt>aaaa</tt>).

  If <b><em>E</em></b> and <b><em>F</em></b> are two regular expressions, you
  can also write <b><em>E</em>|<em>F</em></b> to match either <b><em>E</em></b>
  or <b><em>F</em></b>.  Thus, <b>(min|max|opt)imum</b> matches either
  <tt>minimum</tt>, <tt>maximum</tt> or <tt>optimum</tt>.

  In wildcard mode, there are only four primitives:
  <ul plain>
  <li><b><em>c</em></b> matches the character <tt><em>c</em></tt>
  <li><b>?</b> matches any character
  <li><b>*</b> matches any sequence of characters
  <li><b>[<em>...</em>]</b> matches a defined set of characters (see below)
  </ul>

  QRegExp supports Unicode both in the pattern strings and in the strings to be
  matched.

  When writing regular expressions in C++ code, remember that C++ processes
  <tt>&#92;</tt> characters.  To match a <tt>$</tt> character, you should write
  <tt>"\\$"</tt> in C++ source, not <tt>"\$"</tt>.

  A character set matches a defined set of characters.  For example,
  <b>[BSD]</b> matches any of <tt>B</tt>, <tt>D</tt> and <tt>S</tt>.  Within a
  character set, most special characters lose their usual meaning.  The
  following characters are treated specially:

  <ul plain>
  <li><b>&#92</b> behaves the same way as outside character sets
  <li><b>^</b> negates the caracter set so that it matches any character not in
  the set, when it is placed first in the list
  <li><b>-</b> defines a range of characters
  <li><b>]</b> ends the character set definition.
  </ul>

  Thus, <b>[a-zA-Z0-9.\-]</b> matches upper and lower case ASCII letters,
  digits, dot and hyphen, whereas <b>[^\s]</b> matches everything except white
  space (same as <b>\S</b>).

  \sa QRegExpValidator
*/

#define NumBadChars 128
#define BadChar( ch ) ( (ch).cell() % NumBadChars )

static const int NoOccurrence = INT_MAX;
static const int EmptyCapture = INT_MAX;
static const int InftyLen = INT_MAX;
static const int InftyRep = 1000;
static const int EOS = -1;

static int engCount = 0;
static QArray<int> *noOccurrences = 0;
static QArray<int> *firstOccurrenceAtZero = 0;

/*
  Merges two QArrays of ints and puts the result into the first one.
*/
static void mergeInto( QArray<int> *a, const QArray<int>& b )
{
    int asize = a->size();
    int bsize = b.size();
    if ( asize == 0 ) {
	*a = b.copy();
    } else if ( bsize == 1 && (*a)[asize - 1] < b[0] ) {
	a->resize( asize + 1 );
	(*a)[asize] = b[0];
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
static void reunionInto( QMap<int, int> *a, const QMap<int, int>& b )
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

/*
  Translates a wildcard pattern to an equivalent regular expression pattern
  (e.g., *.cpp to ^.*\.cpp$).
*/
static QString wc2rx( const QString& wc )
{
    int wclen = wc.length();
    QString rx = QChar( '^' );
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
    rx += QChar( '$' );
    return rx;
}

/*
  The class QRegExpEngine encapsulates a modified nondeterministic finite
  automaton (NFA).
*/
class QRegExpEngine : public QShared
{
public:
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
	bool in( ushort ch ) const;
	const QArray<int>& firstOccurrence() const { return occ1; }
#if defined(DEBUG)
	void dump() const;
#endif

    private:
	/*
	  The struct Range represents a range of characters (e.g., [0-9] denotes
	  range 48 to 57.
	*/
	struct Range {
	    ushort from;
	    ushort to;
	};

	int c;
	QArray<Range> r;
	bool n;
	QArray<int> occ1;
    };

    QRegExpEngine( bool caseSensitive ) { setup( caseSensitive ); }
    QRegExpEngine( const QString& rx, bool caseSensitive );
    ~QRegExpEngine();

    bool isValid() const { return valid; }
    bool caseSensitive() const { return cs; }
    int numCaptures() const { return realncap; }
    QArray<int> match( const QString& str, int pos, bool minimal,
		       bool oneTest );
    int matchedLength() const { return mmMatchedLen; }

    int createState( QChar ch );
    int createState( const CharClass& cc );
    int createState( int bref );
    void addCatTransitions( const QArray<int>& from, const QArray<int>& to );
    void addPlusTransitions( const QArray<int>& from, const QArray<int>& to,
			     int atom );
    int anchorAlternation( int a, int b );
    int anchorConcatenation( int a, int b );
    void addAnchors( int from, int to, int a );
    void setupGoodStringHeuristic( int earlyStart, int lateStart,
				   const QString& str );
    void setupBadCharHeuristic( int minLen, const QArray<int>& firstOcc );
    void heuristicallyChooseHeuristic();
#if defined(DEBUG)
    void dump() const;
#endif

private:
    enum { CharClassBit = 0x10000, BackRefBit = 0x20000 };

    /*
      The struct State represents one state in a modified NFA.  The input
      characters matched are stored in the state instead of on the transitions,
      something possible for an automaton constructed from a regular expression.
    */
    struct State {
	int atom;
	int match;
	QArray<int> outs;
	QMap<int, int> *reenter;
	QMap<int, int> *anchors;

	State( int a, int m )
	    : atom( a ), match( m ), reenter( 0 ), anchors( 0 ) { }
	~State() { delete reenter; delete anchors; }
    };

    /*
      The struct Lookahead represents a lookahead a la Perl (e.g., (?=foo) and
      (?!bar)).  The embedded regular expression is represented by an NFA.
    */
    struct Lookahead {
	QRegExpEngine *eng;
	bool neg;

	Lookahead( QRegExpEngine *eng0, bool neg0 )
	    : eng( eng0 ), neg( neg0 ) { }
	~Lookahead() { delete eng; }
    };

    /*
      The struct Atom represents one node in the hierarchy of regular expression
      atoms.
    */
    struct Atom {
	int parent;
	int capture;
    };

    /*
      The struct AnchorAlternation represents a pair of anchors with OR
      semantics.
    */
    struct AnchorAlternation {
	int a;
	int b;
    };

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
    int startAtom( bool capture );
    void finishAtom( int atom );
    int addLookahead( QRegExpEngine *eng, bool negative );

    bool isBetterCapture( const int *begin1, const int *end1, const int *begin2,
			  const int *end2 );
    bool testAnchor( int i, int a, const int *capBegin );

    bool goodStringMatch();
    bool badCharMatch();
    bool testMatch();

    QVector<State> s;
    int ns;
    QArray<Atom> f;
    int nf;
    int cf;
    int realncap;
    int ncap;
    QVector<CharClass> cl;
    QVector<Lookahead> ahead;
    QArray<AnchorAlternation> aa;
    bool caretAnchored;
    bool valid;
    bool cs;
    int nbrefs;

    bool useGoodStringHeuristic;

    // good-string heuristic
    int goodEarlyStart;
    int goodLateStart;
    QString goodStr;

    // bad-character heuristic
    int minl;
    QArray<int> occ1;

    /*
      The class Box is an abstraction for a regular expression fragment.
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
	void set( int backref );

	void cat( const Box& b );
	void orx( const Box& b );
	void plus( int atom );
	void opt();
	void catAnchor( int a );
	void setupHeuristics();
#if defined(DEBUG)
	void dump() const;
#endif

    private:
	void addAnchorsToEngine( const Box& to ) const;

	QRegExpEngine *eng;
	QArray<int> ls;
	QArray<int> rs;
	QMap<int, int> lanchors;
	QMap<int, int> ranchors;
	int skipanchors;

	int earlyStart;
	int lateStart;
	QString str;
	QString leftStr;
	QString rightStr;
	int maxl;

	int minl;
	QArray<int> occ1;
    };

    /*
      This is the lexical analyzer for regular expressions.
    */
    enum { Tok_Eos, Tok_Dollar, Tok_LeftParen, Tok_MagicLeftParen,
	   Tok_PosLookahead, Tok_NegLookahead, Tok_RightParen, Tok_CharClass,
	   Tok_Caret, Tok_Quantifier, Tok_Bar, Tok_Word, Tok_NonWord,
	   Tok_Char = 0x10000, Tok_BackRef = 0x20000 };
    int getChar();
    int getEscape();
    int getRep( int def );
    void startTokenizer( const QChar *rx, int len );
    int getToken();
    void skipChars( int n );

    const QChar *yyIn;
    int yyPos0;
    int yyPos;
    int yyLen;
    int yyCh;
    CharClass *yyCharClass;
    int yyMinRep;
    int yyMaxRep;
    bool yyError;

    /*
      This is the syntactic analyzer for regular expressions.
    */
    int parse( const QChar *rx, int len );
    void parseAtom( Box *box );
    void parseFactor( Box *box );
    void parseTerm( Box *box );
    void parseExpression( Box *box );

    int yyTok;
    bool yyMayCapture;

    /*
      This is the engine state during matching.
    */
    const QString *mmStr;
    const QChar *mmIn;
    int mmStartPos;
    int mmPos;
    int mmLen;
    bool mmMinimal;
    QArray<int> mmCaptured;
    QArray<int> mmCapturedNoMatch;
    QArray<int> mmSuetelane;
    int *mmInNextStack;
    int *mmCurStack;
    int *mmNextStack;
    int *mmCurCapBegin;
    int *mmNextCapBegin;
    int *mmCurCapEnd;
    int *mmNextCapEnd;
    int *mmTempCapBegin;
    int *mmTempCapEnd;
    int *mmCapBegin;
    int *mmCapEnd;
    int *mmSlideTab;
    int mmSlideTabSize;
    QIntDict<int> mmSleeping;
    int mmMatchedLen;
};

QRegExpEngine::QRegExpEngine( const QString& rx, bool caseSensitive )
    : mmSleeping( 101 )
{
    setup( caseSensitive );
    valid = ( parse(rx.unicode(), rx.length()) == (int) rx.length() );
}

QRegExpEngine::~QRegExpEngine()
{
    if ( --engCount == 0 ) {
	delete noOccurrences;
	noOccurrences = 0;
	delete firstOccurrenceAtZero;
	firstOccurrenceAtZero = 0;
    }
}

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
    mmStartPos = pos;
    mmPos = pos;
    mmLen = str.length();
    mmMinimal = minimal;
    mmMatchedLen = -1;

    bool matched = FALSE;
    if ( valid ) {
	if ( mmPos >= 0 && mmPos <= mmLen ) {
	    if ( mmPos <= mmLen - minl ) {
		if ( caretAnchored || oneTest )
		    matched = testMatch();
		else if ( useGoodStringHeuristic )
		    matched = goodStringMatch();
		else
		    matched = badCharMatch();
	    }
	} else {
#if defined(CHECK_RANGE)
	    qWarning( "QRegExpEngine::match: Index %d out of range", mmPos );
#endif
	}
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

int QRegExpEngine::createState( QChar ch )
{
    return setupState( ch.unicode() );
}

int QRegExpEngine::createState( const CharClass& cc )
{
    int n = cl.size();
    cl.resize( n + 1 );
    cl.insert( n, new CharClass(cc) );
    return setupState( CharClassBit | n );
}

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

void QRegExpEngine::addCatTransitions( const QArray<int>& from,
				       const QArray<int>& to )
{
    for ( int i = 0; i < (int) from.size(); i++ ) {
	State *st = s[from[i]];
	mergeInto( &st->outs, to );
    }
}

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
	    for ( int j = to.size() - 1; j >= 0; j-- ) {
		if ( !st->reenter->contains(to[j]) &&
		     oldOuts.bsearch(to[j]) < 0 )
		    st->reenter->insert( to[j], atom );
	    }
	}
    }
}

int QRegExpEngine::anchorAlternation( int a, int b )
{
    if ( a == 0 || b == 0 )
	return 0;
    if ( (a & Anchor_Alternation) == 0 && (b & Anchor_Alternation) == 0 ) {
	if ( (a & b) == a )
	    return a;
	else if ( (a & b) == b )
	    return b;
    }
    int n = aa.size();
    aa.resize( n + 1 );
    aa[n].a = a;
    aa[n].b = b;
    return Anchor_Alternation | n;
}

int QRegExpEngine::anchorConcatenation( int a, int b )
{
    if ( a == 0 )
	return b;
    if ( b == 0 )
	return a;
    if ( (a & Anchor_Alternation) == 0 && (b & Anchor_Alternation) == 0 )
	return a | b;

    if ( (a & Anchor_Alternation) == 0 )
	qSwap( a, b );
    int aprime = anchorConcatenation( aa[a ^ Anchor_Alternation].a, b );
    int bprime = anchorConcatenation( aa[a ^ Anchor_Alternation].b, b );
    return anchorAlternation( aprime, bprime );
}

void QRegExpEngine::addAnchors( int from, int to, int a )
{
    State *st = s[from];
    if ( st->anchors == 0 )
	st->anchors = new QMap<int, int>;
    if ( st->anchors->contains(to) )
	a = anchorAlternation( (*st->anchors)[to], a );
    st->anchors->insert( to, a );
}

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

#if defined(DEBUG)
void QRegExpEngine::dump() const
{
    int i, j;
    qDebug( "Case %ssensitive engine", cs ? "" : "in" );
    qDebug( "  States" );
    for ( i = 0; i < ns; i++ ) {
	qDebug( "  %d in %d%s", i, s[i]->atom,
		i == InitialState ? " (initial)" :
		i == FinalState ? " (final)" : "" );
	int m = s[i]->match;
	if ( (m & CharClassBit) != 0 ) {
	    qDebug( "    match character class %d", m ^ CharClassBit );
	    cl[m ^ CharClassBit]->dump();
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
    if ( nf > 0 ) {
	qDebug( "  Atom    Parent  Capture" );
	for ( i = 0; i < nf; i++ )
	    qDebug( "  %6d  %6d  %6d", i, f[i].parent, f[i].capture );
    }
    for ( i = 0; i < (int) aa.size(); i++ )
	qDebug( "  Anchor alternation 0x%.8x: 0x%.8x 0x%.9x", i, aa[i].a,
		aa[i].b );
}
#endif

void QRegExpEngine::setup( bool caseSensitive )
{
    if ( engCount++ == 0 ) {
	noOccurrences = new QArray<int>( NumBadChars );
	firstOccurrenceAtZero = new QArray<int>( NumBadChars );
	noOccurrences->fill( NoOccurrence );
	firstOccurrenceAtZero->fill( 0 );
    }
    s.setAutoDelete( TRUE );
    s.resize( 32 );
    ns = 0;
    f.resize( 32 );
    nf = 0;
    cf = -1;
    realncap = 0;
    ncap = 0;
    cl.setAutoDelete( TRUE );
    ahead.setAutoDelete( TRUE );
    caretAnchored = TRUE;
    valid = TRUE;
    cs = caseSensitive;
    nbrefs = 0;
    useGoodStringHeuristic = FALSE;
    minl = 0;
    occ1 = *firstOccurrenceAtZero;
}

int QRegExpEngine::setupState( int match )
{
    if ( (ns & (ns + 1)) == 0 && ns + 1 >= s.size() )
	s.resize( (ns + 1) << 1 );
    s.insert( ns, new State(cf, match) );
    return ns++;
}

int QRegExpEngine::startAtom( bool capture )
{
    if ( (nf & (nf + 1)) == 0 && nf + 1 >= f.size() )
	f.resize( (nf + 1) << 1 );
    f[nf].parent = cf;
    cf = nf++;
    f[cf].capture = capture ? ncap++ : -1;
    return cf;
}

void QRegExpEngine::finishAtom( int atom )
{
    cf = f[atom].parent;

    /*
      To save space, we remove needless atoms from the hierarchy.
    */
    if ( nf > 1 && atom == nf - 1 && f[atom].capture < 0 ) {
	nf--;
	for ( int i = ns - 1; i >= 0 && s[i]->atom == atom; i-- )
	    s[i]->atom = f[atom].parent;
    }
}

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

/*
  We want the first longest captures.
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

bool QRegExpEngine::testAnchor( int i, int a, const int *capBegin )
{
    bool catchx = TRUE;
    int j;

    if ( (a & Anchor_Alternation) == 0 ) {
	if ( (a & Anchor_Caret) != 0 ) {
	    if ( mmPos + i != 0 )
		return FALSE;
	}
	if ( (a & Anchor_Dollar) != 0 ) {
	    if ( mmPos + i != mmLen )
		return FALSE;
	}
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
	if ( (a & Anchor_LookaheadMask) != 0 ) {
	    for ( j = 0; j < (int) ahead.size(); j++ ) {
		if ( (a & (Anchor_FirstLookahead << j)) != 0 ) {
		    catchx = ( ahead[j]->eng->match(
				       QConstString((QChar *) mmIn + mmPos + i,
						    mmLen - mmPos - i).string(),
				       0, TRUE, TRUE)[0] == 0 );
		    if ( catchx == ahead[j]->neg )
			return FALSE;
		}
	    }
	}
	for ( j = 0; j < nbrefs; j++ ) {
	    if ( (a & (Anchor_BackRef1Empty << j)) != 0 ) {
		if ( capBegin[j] != EmptyCapture )
		    return FALSE;
	    }
	}
	return TRUE;
    } else {
	return testAnchor( i, aa[a ^ Anchor_Alternation].a, capBegin ) ||
	       testAnchor( i, aa[a ^ Anchor_Alternation].b, capBegin );
    }
}

bool QRegExpEngine::goodStringMatch()
{
    int k = mmPos + goodEarlyStart;

    while ( (k = mmStr->find(goodStr, k, cs)) != -1 ) {
	int from = k - goodLateStart;
	int to = k - goodEarlyStart;
	if ( from > mmPos )
	    mmPos = from;

	while ( mmPos <= to ) {
	    if ( testMatch() )
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

    while ( mmPos <= lastPos ) {
	if ( ++slideNext >= mmSlideTabSize )
	    slideNext = 0;
	if ( mmSlideTab[slideHead] > 0 ) {
	    if ( mmSlideTab[slideHead] - 1 > mmSlideTab[slideNext] )
		mmSlideTab[slideNext] = mmSlideTab[slideHead] - 1;
	    mmSlideTab[slideHead] = 0;
	} else {
	    if ( testMatch() )
		return TRUE;
	}

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
		k -= minl + 1;
	    if ( sk > mmSlideTab[k] )
		mmSlideTab[k] = sk;
	}
	slideHead = slideNext;
	mmPos++;
    }
    return FALSE;
}

/*
  This function is just like a cricket test match:  It's very long.  (It doesn't
  beat tcp_output(), though.)
*/
bool QRegExpEngine::testMatch()
{
    int ncur = 1, nnext = 0;
    int i = 0, j, k, m, n;
    int *zzZ = 0;
    bool match = FALSE;

    mmCurStack[0] = InitialState;
    if ( ncap > 0 ) {
	for ( j = 0; j < ncap; j++ ) {
	    mmCurCapBegin[j] = EmptyCapture;
	    mmCurCapEnd[j] = EmptyCapture;
	}
    }
    mmMatchedLen = -1;

    while ( (ncur > 0 || mmSleeping.count() > 0) && i <= mmLen - mmPos &&
	    !match ) {
	int ch = mmIn[mmPos + i].unicode();
	for ( j = 0; j < ncur; j++ ) {
	    int cur = mmCurStack[j];
	    State *scur = s[cur];
	    QArray<int>& outs = scur->outs;
	    for ( k = 0; k < (int) outs.size(); k++ ) {
		int next = outs[k];
		State *snext = s[next];
		bool in = TRUE;
		int needSomeSleep = 0;

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
			const CharClass *cc = cl[m ^ CharClassBit];
			if ( cs )
			    in = cc->in( ch );
			else if ( cc->negative() )
			    in = cc->in( QChar(ch).lower() ) &&
				 cc->in( QChar(ch).upper() );
			else
			    in = cc->in( QChar(ch).lower() ) ||
				 cc->in( QChar(ch).upper() );
		    } else { /* ( (m & BackRefBit) != 0 ) */
			int bref = m ^ BackRefBit;
			int ell = j * ncap + ( bref - 1 );

			in = bref <= ncap && mmCurCapBegin[ell] != EmptyCapture;
			if ( in ) {
			    if ( cs )
				in = ( mmIn[mmPos + mmCurCapBegin[ell]] == ch );
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
		    }
		}
		/*
		  All is right.  We must now update our data structures.
		*/
		if ( in ) {
		    int *capBegin, *capEnd;

		    /*
		      If the next state was not encountered yet, all is fine.
		    */
		    if ( (m = mmInNextStack[next]) == -1 ) {
			m = nnext++;
			mmNextStack[m] = next;
			mmInNextStack[next] = m;
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
		    }

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
		}
	    }
	}
	/*
	  If we reached the final state, hurray!  Copy the captured zone.
	*/
	if ( ncap > 0 && (m = mmInNextStack[FinalState]) != -1 ) {
	    memcpy( mmCapBegin, mmNextCapBegin + m * ncap, ncap * sizeof(int) );
	    memcpy( mmCapEnd, mmNextCapEnd + m * ncap, ncap * sizeof(int) );
	}

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
	for ( j = 0; j < nnext; j++ )
	    mmInNextStack[mmNextStack[j]] = -1;

	qSwap( mmCurStack, mmNextStack );
	qSwap( mmCurCapBegin, mmNextCapBegin );
	qSwap( mmCurCapEnd, mmNextCapEnd );
	ncur = nnext;
	nnext = 0;
	i++;
    }

    /*
      If minimal matching is enabled, we might have some sleepers left.
    */
    while ( !mmSleeping.isEmpty() ) {
	zzZ = mmSleeping.take( *QIntDictIterator<int>(mmSleeping) );
	delete[] zzZ;
    }

    match = ( mmMatchedLen >= 0 );
    if ( !match )
	mmMatchedLen = i - 1;
    return match;
}

QRegExpEngine::CharClass::CharClass()
    : c( 0 ), n( FALSE ), occ1( *noOccurrences )
{
}

QRegExpEngine::CharClass& QRegExpEngine::CharClass::operator=(
	const CharClass& cc )
{
    c = cc.c;
    r = cc.r;
    n = cc.n;
    occ1 = cc.occ1;
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
    occ1 = *firstOccurrenceAtZero;
}

void QRegExpEngine::CharClass::addCategories( int cats )
{
    c |= cats;
    occ1 = *firstOccurrenceAtZero;
}

void QRegExpEngine::CharClass::addRange( ushort from, ushort to )
{
    int i;
    if ( from > to )
	qSwap( from, to );
    int n = r.size();
    r.resize( n + 1 );
    r[n].from = from;
    r[n].to = to;
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
}

bool QRegExpEngine::CharClass::in( ushort ch ) const
{
    int i;
    if ( c != 0 ) {
	int cat = (int) QChar( ch ).category();
	if ( (c & (1 << cat)) != 0 )
	    return !n;
    }
    for ( i = r.size() - 1; i >= 0; i-- ) {
	if ( ch >= r[i].from && ch <= r[i].to )
	    return !n;
    }
    return n;
}

#if defined(DEBUG)
void QRegExpEngine::CharClass::dump() const
{
    int i;
    qDebug( "    %stive character class", n ? "nega" : "posi" );
    if ( c != 0 )
	qDebug( "      categories 0x%.8x", c );
    for ( i = r.size() - 1; i >= 0; i-- )
	qDebug( "      0x%.4x through 0x%.4x", r[i].from, r[i].to );
}
#endif

QRegExpEngine::Box::Box( QRegExpEngine *engine )
    : eng( engine ), skipanchors( 0 ), earlyStart( 0 ), lateStart( 0 ),
      maxl( 0 ), minl( 0 ), occ1( *noOccurrences )
{
}

QRegExpEngine::Box& QRegExpEngine::Box::operator=( const Box& b )
{
    eng = b.eng;
    ls = b.ls;
    rs = b.rs;
    lanchors = b.lanchors;
    ranchors = b.ranchors;
    skipanchors = b.skipanchors;
    earlyStart = b.earlyStart;
    lateStart = b.lateStart;
    str = b.str;
    leftStr = b.leftStr;
    rightStr = b.rightStr;
    maxl = b.maxl;
    minl = b.minl;
    occ1 = b.occ1;
    return *this;
}

void QRegExpEngine::Box::set( QChar ch )
{
    ls.resize( 1 );
    ls[0] = eng->createState( ch );
    rs = ls;
    str = ch;
    leftStr = ch;
    rightStr = ch;
    maxl = 1;
    minl = 1;
    occ1.detach();
    occ1[BadChar(ch)] = 0;
}

void QRegExpEngine::Box::set( const CharClass& cc )
{
    ls.resize( 1 );
    ls[0] = eng->createState( cc );
    rs = ls;
    maxl = 1;
    minl = 1;
    occ1 = cc.firstOccurrence();
}

void QRegExpEngine::Box::set( int bref )
{
    ls.resize( 1 );
    ls[0] = eng->createState( bref );
    rs = ls;
    if ( bref >= 1 && bref <= MaxBackRefs )
	skipanchors = Anchor_BackRef0Empty << bref;
    maxl = InftyLen;
    minl = 0;
}

void QRegExpEngine::Box::cat( const Box& b )
{
    eng->addCatTransitions( rs, b.ls );
    if ( ranchors.count() + b.lanchors.count() > 0 )
	addAnchorsToEngine( b );
    if ( minl == 0 ) {
	reunionInto( &lanchors, b.lanchors );
	if ( skipanchors != 0 ) {
	    for ( int i = b.ls.size() - 1; i >= 0; i-- ) {
		int a = eng->anchorConcatenation( at(lanchors, b.ls[i]),
						  skipanchors );
		lanchors.insert( b.ls[i], a );
	    }
	}
	mergeInto( &ls, b.ls );
    }
    if ( b.minl == 0 ) {
	reunionInto( &ranchors, b.ranchors );
	if ( b.skipanchors != 0 ) {
	    for ( int i = rs.size() - 1; i >= 0; i-- ) {
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

    minl += b.minl;
    if ( minl == 0 )
	skipanchors = eng->anchorConcatenation( skipanchors, b.skipanchors );
    else
	skipanchors = 0;
}

void QRegExpEngine::Box::orx( const Box& b )
{
    mergeInto( &ls, b.ls );
    reunionInto( &lanchors, b.lanchors );
    mergeInto( &rs, b.rs );
    reunionInto( &ranchors, b.ranchors );
    skipanchors = eng->anchorAlternation( skipanchors, b.skipanchors );
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
    if ( b.minl < minl )
	minl = b.minl;
}

void QRegExpEngine::Box::plus( int atom )
{
    eng->addPlusTransitions( rs, ls, atom );
    addAnchorsToEngine( *this );
    maxl = InftyLen;
}

void QRegExpEngine::Box::opt()
{
    earlyStart = 0;
    lateStart = 0;
    str = QString::null;
    leftStr = QString::null;
    rightStr = QString::null;

    skipanchors = 0;
    minl = 0;
}

void QRegExpEngine::Box::catAnchor( int a )
{
    if ( a != 0 ) {
	for ( int i = rs.size() - 1; i >= 0; i-- ) {
	    a = eng->anchorConcatenation( at(ranchors, rs[i]), a );
	    ranchors.insert( rs[i], a );
	}
	if ( minl == 0 )
	    skipanchors = eng->anchorConcatenation( skipanchors, a );
    }
}

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

#if defined(DEBUG)
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
    for ( int i = to.ls.size() - 1; i >= 0; i-- ) {
	for ( int j = rs.size() - 1; j >= 0; j-- ) {
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
    const char tab[] = "afnrtv"; // no b, as \b means word boundary
    const char backTab[] = "\a\f\n\r\t\v";
    ushort val;
    ushort low;
    int i;
    int prevCh = yyCh;

    if ( prevCh == EOS ) {
	yyError = TRUE;
	return Tok_Char | '\\';
    }
    yyCh = getChar();
    const char *p = strchr( tab, prevCh );
    if ( p != 0 ) {
	return Tok_Char | backTab[p - tab];
    } else {
	switch ( prevCh ) {
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
	case 'B':
	    return Tok_NonWord;
	case 'D':
	    yyCharClass->addCategories( 0x7fffffef );
	    return Tok_CharClass;
	case 'S':
	    yyCharClass->addCategories( 0x7ffff87f );
	    yyCharClass->addRange( 0x0000, 0x0008 );
	    yyCharClass->addRange( 0x000e, 0x001f );
	    yyCharClass->addRange( 0x007f, 0x009f );
	    return Tok_CharClass;
	case 'W':
	    yyCharClass->addCategories( 0x7ff07f8f );
	    return Tok_CharClass;
	case 'b':
	    return Tok_Word;
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
	default:
	    if ( prevCh >= '1' && prevCh <= '9' ) {
		val = prevCh - '0';
		while ( yyCh >= '0' && yyCh <= '9' ) {
		    val = ( val *= 10 ) | ( yyCh - '0' );
		    yyCh = getChar();
		}
		return Tok_BackRef | val;
	    } else {
		return Tok_Char | prevCh;
	    }
	}
    }
}

int QRegExpEngine::getRep( int def )
{
    if ( yyCh >= '0' && yyCh <= '9' ) {
	int rep = 0;
	do {
	    rep = ( 10 * rep ) + ( yyCh - '0' );
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
    ushort pendingCh = 0;
    bool charPending;
    bool rangePending;
    int tok;
    int prevCh = yyCh;

    yyPos0 = yyPos - 1;
    yyCharClass->clear();
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
	    case '!':
		return Tok_NegLookahead;
	    case '=':
		return Tok_PosLookahead;
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
	yyCharClass->setNegative( TRUE );
	return Tok_CharClass;
    case '?':
	yyMinRep = 0;
	yyMaxRep = 1;
	return Tok_Quantifier;
    case '[':
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
    case '\\':
	return getEscape();
    case ']':
	yyError = TRUE;
	return Tok_Char | ']';
    case '^':
	return Tok_Caret;
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
    case '|':
	return Tok_Bar;
    case '}':
	yyError = TRUE;
	return Tok_Char | '}';
    default:
	return Tok_Char | prevCh;
    }
}

void QRegExpEngine::skipChars( int n )
{
    if ( n > 0 ) {
	yyPos += n - 1;
	yyCh = getChar();
    }
}

int QRegExpEngine::parse( const QChar *pattern, int len )
{
    startTokenizer( pattern, len );
    yyTok = getToken();
    yyMayCapture = TRUE;

    int atom = startAtom( FALSE );
    CharClass anything;
    Box box( this ); // create InitialState
    box.set( anything );
    Box rightBox( this ); // create FinalState
    rightBox.set( anything );
    Box middleBox( this );
    parseExpression( &middleBox );
    finishAtom( atom );
    middleBox.setupHeuristics();
    box.cat( middleBox );
    box.cat( rightBox );
    delete yyCharClass;
    yyCharClass = 0;

    realncap = ncap;
    if ( nbrefs > ncap )
	ncap = nbrefs;

    mmCaptured.resize( 2 + 2 * realncap );
    mmCapturedNoMatch.fill( -1, 2 + 2 * realncap );

    /*
      We use one QArray<int> for all the big data used a lot in testMatch() and
      friends.
    */
    mmSlideTabSize = QMAX( minl + 1, 16 );
    mmSuetelane.resize( (3 + 4 * ncap) * ns + 4 * ncap + mmSlideTabSize );

    mmInNextStack = mmSuetelane.data();
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

    State *sinit = s[InitialState];
    caretAnchored = ( sinit->anchors != 0 );
    if ( caretAnchored ) {
	QMap<int, int>& anchors = *sinit->anchors;
	QMap<int, int>::ConstIterator a;
	for ( a = anchors.begin(); a != anchors.end(); ++a ) {
	    if ( (*a & Anchor_Alternation) != 0 )
		break;
	    if ( (*a & Anchor_Caret) == 0 ) {
		caretAnchored = FALSE;
		break;
	    }
	}
    }
    return yyPos0;
}

void QRegExpEngine::parseAtom( Box *box )
{
    QRegExpEngine *eng = 0;
    bool neg;
    int len;

    switch ( yyTok ) {
    case Tok_Dollar:
	box->catAnchor( Anchor_Dollar );
	break;
    case Tok_Caret:
	box->catAnchor( Anchor_Caret );
	break;
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
    case Tok_Word:
	box->catAnchor( Anchor_Word );
	break;
    case Tok_NonWord:
	box->catAnchor( Anchor_NonWord );
	break;
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
	else if ( (yyTok & Tok_BackRef) != 0 )
	    box->set( yyTok ^ Tok_BackRef );
	else
	    yyError = TRUE;
    }
    yyTok = getToken();
}

void QRegExpEngine::parseFactor( Box *box )
{
#define YYREDO() \
	yyIn = in, yyPos0 = pos0, yyPos = pos, yyLen = len, yyCh = ch, \
	*yyCharClass = charClass, yyMinRep = 0, yyMaxRep = 0, yyTok = tok

    int i;
    int atom = startAtom( yyMayCapture && yyTok == Tok_LeftParen );
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

    parseAtom( box );
    finishAtom( atom );

    if ( yyTok == Tok_Quantifier ) {
	if ( yyMaxRep == 0 )
	    box->clear();
	else if ( yyMaxRep == InftyRep )
	    box->plus( atom );

	if ( yyMinRep == 0 )
	    box->opt();

	yyMayCapture = FALSE;
	int alpha = ( yyMinRep == 0 ) ? 0 : yyMinRep - 1;
	int beta = ( yyMaxRep == InftyRep ) ? 0 : yyMaxRep - ( alpha + 1 );

	Box rightBox( this );
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
	yyTok = getToken();
	yyMayCapture = mayCapture;
    }
#undef YYREDO
}

void QRegExpEngine::parseTerm( Box *box )
{
    if ( yyTok != Tok_Eos && yyTok != Tok_RightParen && yyTok != Tok_Bar )
	parseFactor( box );
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
struct QRegExpPrivate {
    QString pattern;
    bool wc;
    bool min;
    QString t;
    QStringList capturedCache;
    QArray<int> captured;

    QRegExpPrivate() { captured.fill( -1, 2 ); }
};

/*!  Constructs an invalid regular expression (which never matches).

  \sa isValid()
*/

QRegExp::QRegExp()
{
    eng = new QRegExpEngine( TRUE );
    priv = new QRegExpPrivate;
    priv->wc = FALSE;
    priv->min = FALSE;
}

/*!  Constructs a regular expression object for a given \a pattern string and
  sets the case sensitivity, wildcard and minimal matching options.

  \sa setPattern() setCaseSensitive() setWildcard() setMinimal()
*/

QRegExp::QRegExp( const QString& pattern, bool caseSensitive, bool wildcard,
		  bool minimal )
{
    eng = new QRegExpEngine( TRUE );
    priv = new QRegExpPrivate;
    priv->pattern = pattern;
    priv->wc = wildcard;
    priv->min = minimal;
    compile( caseSensitive );
}

/*!  Constructs a regular expression as a copy of \a rx.

  \sa operator=()
*/

QRegExp::QRegExp( const QRegExp& rx )
{
    eng = new QRegExpEngine( TRUE );
    priv = new QRegExpPrivate;
    operator=( rx );
}

/*!  Destructs the regular expression and cleans up its internal data.
*/

QRegExp::~QRegExp()
{
    if ( eng->deref() )
	delete eng;
    delete priv;
}

/*!  Copies the regular expression \a rx and returns a reference to this
  QRegExp.  The case sensitivity, wildcard and minimal matching options are
  copied as well.
*/

QRegExp& QRegExp::operator=( const QRegExp& rx )
{
    rx.eng->ref();
    if ( eng->deref() )
	delete eng;
    eng = rx.eng;
    priv->pattern = rx.priv->pattern;
    priv->wc = rx.priv->wc;
    priv->min = rx.priv->min;
    priv->t = rx.priv->t;
    priv->capturedCache = rx.priv->capturedCache;
    priv->captured = rx.priv->captured;
    return *this;
}

/*!  Returns TRUE if this regular expression is equal to \a rx, otherwise FALSE.

  Two QRegExp objects are equal if they have equal pattern strings, and if case
  sensitivity, wildcard and minimal matching options are identical.
*/

bool QRegExp::operator==( const QRegExp& rx ) const
{
    return priv->pattern == rx.priv->pattern &&
	   eng->caseSensitive() == rx.eng->caseSensitive() &&
	   priv->wc == rx.priv->wc &&
	   priv->min == rx.priv->min;
}

/*!  \fn bool QRegExp::operator!=( const QRegExp& rx ) const

  Returns TRUE if this regular expression is not equal to \a rx, otherwise
  FALSE.

  \sa operator==()
*/

/*!  \fn bool QRegExp::isValid() const

  Returns TRUE if the regular expression is valid, or FALSE if it's invalid.  An
  invalid regular expression never matches.

  The pattern <b>[a-z</b> is an example of an invalid pattern, since it lacks
  a closing bracket.
*/

bool QRegExp::isValid() const
{
    return eng->isValid();
}

/*!  Returns the pattern string of the regular expression.

  \sa setPattern()
*/

QString QRegExp::pattern() const
{
    return priv->pattern;
}

/*!  Sets the pattern string to \a pattern and returns a reference to this
  regular expression.  The case sensitivity, wildcard and minimal matching
  options are left alone.

  \sa pattern()
*/

void QRegExp::setPattern( const QString& pattern )
{
    if ( priv->pattern != pattern ) {
	priv->pattern = pattern;
	compile( caseSensitive() );
    }
}

/*!  Returns TRUE if case sensitivity is enabled, otherwise FALSE.

  \sa setCaseSensitive()
*/

bool QRegExp::caseSensitive() const
{
    return eng->caseSensitive();
}

/*!  Enables or disables case sensitive matching.  The default is TRUE.

  If case sensitivity is on, <b>T.X</b> matches <tt>TeX</tt> but not
  <tt>Tex</tt>.

  \sa caseSensitive()
*/

void QRegExp::setCaseSensitive( bool sensitive )
{
    if ( sensitive != eng->caseSensitive() )
	compile( sensitive );
}

/*!  Returns TRUE if wildcard mode is enabled, otherwise FALSE.  The default is
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

/*!  Returns TRUE if minimal matching is enabled, otherwise FALSE.

  \sa setMinimal()
*/

bool QRegExp::minimal() const
{
    return priv->min;
}

/*!  Enables or disables minimal matching.  The default is disabled, i.e.,
  \e maximal matching.

  For example, for regular expression <b>\{.*\}</b> and input string
  <b>a{b}c{d}e</b>, the minimal match is <b>{b}</b> and the maximal match is
  <b>{b}c{d}</b>.

  \sa minimal()
*/

void QRegExp::setMinimal( bool minimal )
{
    priv->min = minimal;
}

/*!  Attempts to match in \a str, starting from position \a start.
  Returns the position of the first match, or -1 if there was no match.

  Example:
  \code
    QRegExp rx( "[0-9]*\\.[0-9]+" );    // matches floating point
    int pos = r.match( "pi = 3.14" );   // pos = 5
    int len = r.matchedLength();        // len = 4
  \endcode

  You might prefer to use QString::find(), QString::contains(), or even
  QStringList::grep().

  \sa matchRev() partialMatch() matchedLength() capturedText()
*/

int QRegExp::match( const QString& str, int start )
{
    priv->t = str;
    priv->capturedCache.clear();
    priv->captured = eng->match( str, start, priv->min, FALSE );
    return priv->captured[0];
}

/*!  Attempts to match backwards in \a str, starting at position \a start.

  Returns the position of the first match, or -1 if there was no match.

  You might prefer to use QString::findRev().

  \sa match() partialMatch() matchedLength() capturedText()
*/

int QRegExp::matchRev( const QString& str, int start )
{
    priv->t = str;
    priv->capturedCache.clear();
    if ( start < 0 ) {
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

/*!  Returns TRUE if \a str is a prefix of a string matched exactly by this
  regular expression.

  For example, if the regular expression is <b>abc</b>, then this function
  returns TRUE for <b>abc</b> or <b>ab</b>, but FALSE for <b>hab</b>.

  \sa match() matchRev()
*/

bool QRegExp::partialMatch( const QString& str ) const
{
    priv->t = QString::null;
    priv->capturedCache.clear();
    priv->captured.detach();
    priv->captured.fill( -1 );

    eng->match( str, 0, priv->min, TRUE );
    return eng->matchedLength() == (int) str.length();
}

/*!  Returns the length of the matched string, or -1 if there was no match.

  \sa match() matchRev() setMinimal()
*/

int QRegExp::matchedLength() const
{
    return priv->captured[1];
}

/*!  Returns the text captured by the \a nth parenthesized sub-expression in the
  regular expression.  Sub-expressions are numered in the order of occurrence of
  their left parenthesis, starting at 1.  The whole regular expression is given
  number 0, making it possible to retrieve the full matched text easily.

  Example 1:
  \code
    QRegExp rx( "a*((b*)y*)(z*)" );     // match a...ab...by...yz...z
    int pos = rx.match( "abbyyyz" );    // pos = 0
    QString t0 = rx.capturedText( 0 );  // abbyyyz
    QString t1 = rx.capturedText( 1 );  // bbyyy
    QString t2 = rx.capturedText( 2 );  // bb
    QString t3 = rx.capturedText( 3 );  // z
  \endcode

  Example 2:
  \code
    QRegExp rx( "([a-z])+" );           // match a lower-case word
    int pos = rx.match( "3 pizza 2" );  // pos = 4
    QString t0 = rx.capturedText( 0 );  // pizza
    QString t1 = rx.capturedText( 1 );  // a
  \endcode

  Notice the behavior of sub-expressions governed by a quantifier (<b>+</b>).

  \sa capturedTexts() setMinimal()
*/

QString QRegExp::capturedText( int nth ) const
{
    if ( nth < 0 || nth >= (int) priv->captured.size() / 2 ) {
#if defined(CHECK_RANGE)
	qWarning( "QRegExp::capturedText: Index %d out of range", nth );
#endif
	return QString::null;
    }
    return capturedTexts()[nth];
}

/*!  Returns a list of the captured text strings.

  \sa capturedText() setMinimal()
*/

QStringList QRegExp::capturedTexts() const
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

void QRegExp::compile( bool caseSensitive )
{
    if ( eng->deref() )
	delete eng;
    if ( priv->pattern.isNull() )
	eng = new QRegExpEngine( caseSensitive );
    else
	eng = new QRegExpEngine( priv->wc ? wc2rx(priv->pattern)
				 : priv->pattern, caseSensitive );
    priv->t = QString::null;
    priv->capturedCache.clear();
    priv->captured.detach();
    priv->captured.fill( -1, 2 + 2 * eng->numCaptures() );
}
