/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*
  This file is a self-contained interactive indenter for C++,
  and similar programming languages.
*/

#include <qregexp.h>

/*
  The indenter avoids getting stuck in almost infinite loops by
  imposing arbitrary limits on the number of lines it analyzes when
  looking for a construct.

  For example, the indenter never considers more than BigRoof lines
  backwards when looking for the start of a C-style comment.
*/
static const int SmallRoof = 40;
static const int BigRoof = 400;

/*
  The indenter supports a few parameters:

    * ppHardwareTabSize is the size of a '\t' in your favorite editor.
    * ppIndentSize is the size of an indentation, the "software tab
      size".
    * ppContinuationIndentSize is the extra indent for a continuation
      line, when there is nothing to align against on the previous
      line.
    * ppCommentOffset is the indentation within a C-style comment,
      when it cannot be picked up.
*/
static int ppHardwareTabSize = 8;
static int ppIndentSize = 4;
static int ppContinuationIndentSize = 8;
static int ppCommentOffset = 2;

/*
  'do', 'for', etc., are the keywords that can appear in the following
  construct:

      keyword ( x )
	  y;
*/
static QRegExp ctlStmtKeyword( QString("\\b(?:do|for|if|while)\\b") );

static QRegExp braceElse( QString("^\\s*\\}\\s*else\\b") );
static QRegExp forKeyword( QString("\\bfor\\b") );
static QRegExp label( QString("^\\s*((?:case\\b[^:]+|[a-zA-Z_0-9]+):(?!:))") );

static QString comparators( "!=<>" );
static QString parens( "()" );
static QString separators( "{};" );

static QString slashAster( "/*" );
static QString asterSlash( "*/" );
static QString slashSlash( "//" );

/*
  Returns the first non-space character in the string t, or
  QChar::null if the string is made only of white space.
*/
static QChar firstNonWhiteSpace( const QString& t )
{
    int i = 0;
    while ( i < (int) t.length() ) {
	if ( !t[i].isSpace() )
	    return t[i];
	i++;
    }
    return QChar::null;
}

/*
  Returns TRUE if string t is made only of white space; otherwise
  returns FALSE.
*/
static bool isOnlyWhiteSpace( const QString& t )
{
    return firstNonWhiteSpace( t ).isNull();
}

/*
  Assuming string t is a line, returns the column number of a given
  index. Column numbers and index are identical for strings that don't
  contain '\t's. The index may be t.length().
*/
static int columnForIndex( const QString& t, int index )
{
    int col = 0;
    if ( index > (int) t.length() )
	index = t.length();

    for ( int i = 0; i < index; i++ ) {
	if ( t[i] == QChar('\t') ) {
	    col = ( (col / ppHardwareTabSize) + 1 ) * ppHardwareTabSize;
	} else {
	    col++;
	}
    }
    return col;
}

/*
  Returns the indentation size of string t.
*/
static int indentOfLine( const QString& t )
{
    return columnForIndex( t, t.find(firstNonWhiteSpace(t)) );
}

/*
  Replaces t[k] by ch, unless t[k] is '\t'. Tab characters are better
  left alone since they break the "index equals column" rule. No
  provisions are taken against '\n' or '\r', which shouldn't occur in
  t anyway.
*/
static inline void eraseChar( QString& t, int k, QChar ch )
{
    if ( t[k] != '\t' )
	t[k] = ch;
}

/*
  Removes some nefast constructs from a code line and returns the
  resulting line.
*/
static QString trimmedCodeLine( const QString& t )
{
    static QRegExp literal( QString("([\"'])(?:[^\"\\\\]|\\\\.)*\\1") );

    static QRegExp inlineCComment( "/\\*.*\\*/" );
    inlineCComment.setMinimal( TRUE );

    int k;

    QString trimmed = t;

    /*
      Replace character and string literals by X's, since they may
      contain confusing characters (such as '{' and ';'). "Hello!" is
      replaced by XXXXXXXX. The literals are rigourously of the same
      length before and after; otherwise, we would break alignment of
      continuation lines.
    */
    k = 0;
    while ( (k = trimmed.find(literal, k)) != -1 ) {
	for ( int i = 0; i < literal.matchedLength(); i++ )
	    eraseChar( trimmed, k + i, QChar('X') );
	k += literal.matchedLength();
    }

    /*
      Replace inline C-style comments by spaces. Other comments are
      handled elsewhere.
    */
    k = 0;
    while ( (k = trimmed.find(inlineCComment, k)) != -1 ) {
	for ( int i = 0; i < inlineCComment.matchedLength(); i++ )
	    eraseChar( trimmed, k + i, QChar(' ') );
	k += inlineCComment.matchedLength();
    }

    /*
      Replace case and goto labels by spaces, to allow esoteric
      alignments:

	  foo1: foo2: bar1;
		      bar2;
    */
    while ( trimmed.findRev(QChar(':')) != -1 && trimmed.find(label) != -1 ) {
	QString cap1 = label.cap( 1 );
	int pos1 = label.pos( 1 );
	for ( int i = 0; i < (int) cap1.length(); i++ )
	    eraseChar( trimmed, pos1 + i, QChar(' ') );
    }

    /*
      Remove C++-style comments.
    */
    k = trimmed.find( slashSlash );
    if ( k != -1 )
	trimmed.truncate( k );

    return trimmed;
}

/*
  Returns '(' if the last parenthesis is opening, ')' if it is
  closing, and QChar::null if there are no parentheses in t.
*/
static inline QChar lastParen( const QString& t )
{
    int i = t.length();
    while ( i > 0 ) {
	i--;
	if ( parens.find(t[i]) != -1 )
	    return t[i];
    }
    return QChar::null;
}

/*
  Returns TRUE if typedIn the same as okayCh or is null; otherwise
  returns FALSE.
*/
static inline bool okay( QChar typedIn, QChar okayCh )
{
    return typedIn == QChar::null || typedIn == okayCh;
}

/*
  The "linizer" is a group of functions and variables to iterate
  through the source code of the program to indent. The program is
  given as a list of strings, with the bottom line being the line to
  indent. The actual program might contain extra lines, but those are
  uninteresting and not passed over to us.
*/

struct LinizerState
{
    QStringList::ConstIterator iter;
    QString line;
    int braceDepth;
    bool inCComment;
    bool pendingRightBrace;
};

static QStringList yyProgram;
static LinizerState yyLinizerState;
static const QString& yyLine = yyLinizerState.line;
static const int& yyBraceDepth = yyLinizerState.braceDepth;

/*
  Saves and restores the state of the global linizer. This allows us
  to backtrack gracefully.
*/
#define YY_SAVE() \
	LinizerState savedState = yyLinizerState
#define YY_RESTORE() \
	yyLinizerState = savedState

/*
  Advances to the previous line in yyProgram and update yyLine
  accordingly. yyLine is cleaned from comments and other damageable
  constructs. Empty lines are skipped.
*/
static bool readLine()
{
    int k;

    do {
	if ( yyLinizerState.iter == yyProgram.begin() ) {
	    yyLinizerState.line = QString::null;
	    return FALSE;
	}

	--yyLinizerState.iter;
	yyLinizerState.line = *yyLinizerState.iter;

	yyLinizerState.line = trimmedCodeLine( yyLinizerState.line );

	/*
	  Remove C-style comments that span multiple lines. If the
	  bottom line starts in a C-style comment, we are not aware
	  of that and eventually yyLine will contain a slash-aster.

	  Notice that both if's can be executed, since
	  yyLinizerState.inCComment is potentially set to FALSE in
	  the first if. The order of the if's is also important.
	*/

	if ( yyLinizerState.inCComment ) {
	    k = yyLinizerState.line.find( slashAster );
	    if ( k == -1 ) {
		yyLinizerState.line = QString::null;
	    } else {
		yyLinizerState.line.truncate( k );
		yyLinizerState.inCComment = FALSE;
	    }
	}

	if ( !yyLinizerState.inCComment ) {
	    k = yyLinizerState.line.find( asterSlash );
	    if ( k != -1 ) {
		for ( int i = 0; i < k + 2; i++ )
		    eraseChar( yyLinizerState.line, i, QChar(' ') );
		yyLinizerState.inCComment = TRUE;
	    }
	}

	/*
	  Remove preprocessor directives.
	*/
	k = 0;
	while ( k < (int) yyLinizerState.line.length() ) {
	    QChar ch = yyLinizerState.line[k];
	    if ( ch == QChar('#') ) {
		yyLinizerState.line = QString::null;
	    } else if ( !ch.isSpace() ) {
		break;
	    }
	    k++;
	}

	/*
	  Remove trailing spaces.
	*/
	k = yyLinizerState.line.length();
	while ( k > 0 && yyLinizerState.line[k - 1].isSpace() )
	    k--;
	yyLinizerState.line.truncate( k );

	/*
	  '}' increment the brace depth and '{' decrements it and not
	  the other way around, as we are parsing backwards.
	*/
	yyLinizerState.braceDepth +=
		yyLinizerState.line.contains( QChar('}') ) -
		yyLinizerState.line.contains( QChar('{') );

	/*
	  We use a dirty trick for

	      } else ...

	  We don't count the '}' yet, so that it's more or less
	  equivalent to the friendly construct

	      }
	      else ...
	*/
	if ( yyLinizerState.pendingRightBrace )
	    yyLinizerState.braceDepth++;
	yyLinizerState.pendingRightBrace =
		( yyLinizerState.line.find(braceElse) == 0 );
	if ( yyLinizerState.pendingRightBrace )
	    yyLinizerState.braceDepth--;
    } while ( yyLinizerState.line.isEmpty() );

    return TRUE;
}

/*
  Resets the linizer to its initial state, with yyLine containing the
  line above the bottom line of the program.

  The algorithm we use is multi-pass, so it's handy to be able to
  be able to restart the linizer at any time.
*/
static void restartLinizer()
{
    yyLinizerState.iter = yyProgram.end();
    --yyLinizerState.iter;
    yyLinizerState.braceDepth = 0;
    yyLinizerState.inCComment = FALSE;
    yyLinizerState.pendingRightBrace = FALSE;
    readLine();
}

/*
  Returns TRUE if the start of the bottom line of yyProgram (and
  potentially the whole line) is part of a C-style comment; otherwise
  returns FALSE.
*/
static bool bottomLineStartsInCComment()
{
    /*
      We could use the linizer here, but that would slow us down
      terribly. We are better to trim only the code lines we need.
    */
    QStringList::ConstIterator p = yyProgram.end();
    --p; // skip bottom line

    for ( int i = 0; i < BigRoof; i++ ) {
	if ( p == yyProgram.begin() )
	    return FALSE;
	--p;

	if ( (*p).find(slashAster) != -1 || (*p).find(asterSlash) != -1 ) {
	    QString trimmed = trimmedCodeLine( *p );

	    if ( trimmed.find(slashAster) != -1 ) {
		return TRUE;
	    } else if ( trimmed.find(asterSlash) != -1 ) {
		return FALSE;
	    }
	}
    }
    return FALSE;
}

/*
  Returns the recommended indent for the bottom line of yyProgram
  assuming that it starts in a C-style comment, a condition that is
  tested elsewhere.
*/
static int indentWhenBottomLineStartsInCComment()
{
    static QString slashAster( "/*" );

    restartLinizer();
    for ( int i = 0; i < SmallRoof; i++ ) {
	int k = yyLine.findRev( slashAster );
	if ( k == -1 ) {
	    /*
	      We found a normal text line in a comment. Align the
	      bottom line with the text on this line.
	    */
	    return indentOfLine( yyLine );
	} else {
	    /*
	      The C-style comment starts on this line. If there is
	      text on the same line, align with it. Otherwise, align
	      with the slash-aster plus a given offset.
	    */
	    int indent = columnForIndex( yyLine, k );
	    k += 2;
	    while ( k < (int) yyLine.length() ) {
		if ( !yyLine[k].isSpace() )
		    return columnForIndex( yyLine, k );
		k++;
	    }
	    return indent + ppCommentOffset;
	}

	if ( !readLine() )
	    break;
    }
    return 0;
}

/*
  A function called match*() modifies the linizer state. If it
  returns TRUE, yyLine is the top line of the matched construct;
  otherwise, the linizer is left in an unknown state.

  A function called is*() keeps linizer state intact.
*/

/*
  Returns TRUE if the current line (and upwards) forms a braceless
  control statement; otherwise returns FALSE.

  The first line of the following example is a "braceless control
  statement":

      if ( x )
	  y;
*/
static bool matchBracelessControlStatement()
{
    int delimDepth = 0;

    if ( yyLine.endsWith(QString("else")) )
	return TRUE;

    if ( !yyLine.endsWith(QChar(')')) )
	return FALSE;

    for ( int i = 0; i < SmallRoof; i++ ) {
	int j = yyLine.length();
	while ( j > 0 ) {
	    j--;
	    QChar ch = yyLine[j];

	    switch ( ch.unicode() ) {
	    case ')':
		delimDepth++;
		break;
	    case '(':
		delimDepth--;
		if ( delimDepth == 0 ) {
		    if ( yyLine.find(ctlStmtKeyword) != -1 ) {
			/*
			  We have

			      if ( x )
				  y

			  "if ( x )" is not part of the statement
			  "y".
			*/
			return TRUE;
		    }
		}
		if ( delimDepth == -1 ) {
		    /*
		      We have

			  if ( (1 +
				2)

		      and not

			  if ( 1 +
			       2 )
		    */
		    return FALSE;
		}
		break;
	    case '{':
	    case '}':
	    case ';':
		/*
		  We met a statement separator, but not where we
		  expected it. What follows is probably a weird
		  continuation line. Be careful with ';' in for,
		  though.
		*/
		if ( ch != QChar( ';' ) || delimDepth == 0 )
		    return FALSE;
	    }
	}

	if ( !readLine() )
	    break;
    }
    return FALSE;
}

/*
  Returns TRUE if yyLine is a continuation line; otherwise returns
  FALSE. This function is useful for going up to the first line of a
  statement.

  The functions isContinuationLine() and indentForContinuationLine()
  should have, as much as possible, the same formal definition of
  what is a continuation line.

  In many places, we'll use the terms "standalone line", "unfinished
  line" and "continuation line". The meaning of these should be
  self-evident after considering an example:

      a = b;	// standalone line
      c = d +	// unfinished line
	  e +	// unfinished continuation line
	  f;	// continuation line
*/
static bool isContinuationLine()
{
    bool cont = FALSE;

    YY_SAVE();

    if ( readLine() ) {
	QChar last = yyLine[ (int)yyLine.length() - 1 ];
	if ( separators.find(last) == -1 ) {
	    /*
	      It doesn't end with ';' or similar. If it's not
	      "if ( x )", it must be an unfinished line.
	    */
	    cont = !matchBracelessControlStatement();
	} else if ( last == QChar(';') ) {
	    if ( lastParen(yyLine) == QChar('(') ) {
		/*
		  Exceptional case:

		      for ( int i = 1; i < 10;
			    i++ )
		*/
		cont = TRUE;
	    } else if ( readLine() && last == QChar(';') &&
			lastParen(yyLine) == QChar('(') ) {
		/*
		  Exceptional case:

		      for ( int i = 1;
			    i < 10;
			    i++ )
		*/
		cont = TRUE;
	    }
	}
    }

    YY_RESTORE();
    return cont;
}

static const int NotAContinuationLine = -1;

/*
  Returns the recommended indent for the bottom line of yyProgram if
  it follows an unfinished line; otherwise returns
  NotAContinuationLine.

  See also isContinuationLine().
*/
static int indentForContinuationLine()
{
    int braceDepth = 0;
    int delimDepth = 0;

    restartLinizer();
    if ( yyLine.isEmpty() )
	return NotAContinuationLine;

    for ( int i = 0; i < SmallRoof; i++ ) {
	int hook = -1;

	if ( yyLine.endsWith(QChar(';')) ) {
	    if ( lastParen(yyLine) != QChar('(') ) {
		/*
		  The previous line ends with a ';' and is not the
		  exceptional case

		      for ( int i = 1; i < 10;
			    i++ )
		*/
		YY_SAVE();
		readLine();
		if ( !yyLine.endsWith(QChar(';')) ||
		     lastParen(yyLine) != QChar('(') ||
		     yyLine.find(forKeyword) == -1 ) {
		    /*
		      And it isn't the exceptional case

			  for ( int i = 1;
				i < 10;
				i++ )
		    */
		    return NotAContinuationLine;
		}
		YY_RESTORE();
	    }
	} else if ( yyLine.endsWith(QString("else")) ) {
	    // ideally "\\belse"
	    return NotAContinuationLine;
	}

	int j = yyLine.length();
	while ( j > 0 && hook < 0 ) {
	    j--;
	    QChar ch = yyLine[j];

	    switch ( ch.unicode() ) {
	    case ')':
	    case ']':
		delimDepth++;
		break;
	    case '}':
		braceDepth++;
		break;
	    case '(':
	    case '[':
		delimDepth--;
		/*
		  An unclosed delimiter is a good place to align at,
		  at least for some styles (including Trolltech's).
		*/
		if ( delimDepth == -1 )
		    hook = j;
		break;
	    case '{':
		braceDepth--;
		/*
		  A left brace followed by other stuff on the same
		  line is typically for an enum or an initializer.
		  Such a brace must be treated just like the other
		  delimiters.
		*/
		if ( braceDepth == -1 ) {
		    if ( j < (int) yyLine.length() - 1 ) {
			hook = j;
		    } else {
			return NotAContinuationLine;
		    }
		}
		break;
	    case '=':
		/*
		  An equal sign is a very natural alignment hook
		  because it's usually the operator with the lowest
		  precedence in statements it appears in. Case in
		  point:

		      int x = 1 +
			      2;

		  However, we have to beware of constructs such as
		  default arguments and explicit enum constant
		  values:

		      void foo( int x = 0,
		                int y = 0 );

		  And not

		      void foo( int x = 0,
		                      int y = 0 );

		  These constructs are caracterized by a ',' at the
		  end of the unfinished lines or by non-balanced
		  parentheses.
		*/
		if ( j == 0 || comparators.find(yyLine[j - 1]) == -1 ) {
		    if ( braceDepth == 0 && delimDepth == 0 &&
			 j < (int) yyLine.length() - 1 &&
			 yyLine.contains(QChar(',')) == 0 &&
			 (yyLine.contains(QChar('(')) ==
			  yyLine.contains(QChar(')'))) )
			hook = j;
		}
	    }
	}

	if ( hook >= 0 ) {
	    /*
	      Yes, we have a delimiter or an operator to align
	      against! We don't really align against it, but rather
	      against the following token, if any. In this example,
	      the following token is "11":

		  int x = ( 11 +
			    2 );

	      If there is no such token, put a continuation indent:

		  static QRegExp foo( QString(
			  "foo foo foo foo foo foo foo foo foo") );
	    */
	    hook++;
	    while ( hook < (int) yyLine.length() ) {
		if ( !yyLine[hook].isSpace() )
		    return columnForIndex( yyLine, hook );
		hook++;
	    }
	    return indentOfLine( yyLine ) + ppContinuationIndentSize;
	}

	if ( braceDepth != 0 )
	    break;

	/*
	  The line's delimiters are balanced. It looks like a
	  continuation line or something. We could be wrong, of
	  course.
	*/
	if ( delimDepth == 0 ) {
	    if ( yyLine.find(ctlStmtKeyword) != -1 ) {
		/*
		  The line is of the form "if ( x )". The following
		  code line is clearly not a continuation line.
		*/
		return NotAContinuationLine;
	    } else if ( isContinuationLine() ) {
		/*
		  We have

		      x = 1 +
			  2 +
			  3;

		  The "3;" should fall right under the "2;".
		*/
		return indentOfLine( yyLine );
	    } else {
		/*
		  We have

		      stream << 1 +
			      2;

		  We could, but we don't, try to analyze which
		  operator has precedence over which and so on, in
		  which case we could give the excellent result

		      stream << 1 +
				2;

		  (We do have a special trick above for the
		  assignment operator above, though.)
		*/
		return indentOfLine( yyLine ) + ppContinuationIndentSize;
	    }
	}

	if ( !readLine() )
	    break;
    }
    return NotAContinuationLine;
}

/*
  Returns the recommended indent for the bottom line of yyProgram if
  that line is standalone (or should be indented likewise).
*/
static int indentForStandaloneLine()
{
    bool followedByLeftBrace = FALSE;

    restartLinizer();
    for ( int i = 0; i < SmallRoof; i++ ) {
	if ( !followedByLeftBrace ) {
	    YY_SAVE();

	    if ( matchBracelessControlStatement() ) {
		/*
		  The situation is this, and we want to indent "z;":

		      if ( x &&
		           y )
			  z;

		  yyLine is "if ( x &&".
		*/
		return indentOfLine( yyLine ) + ppIndentSize;
	    }
	    YY_RESTORE();
	}

	if ( yyLine.endsWith(QChar(';')) || yyLine.contains(QChar('{')) > 0 ) {
	    /*
	      The situation is possibly this, and we want to indent
	      "z;":

		  while ( x )
		      y;
		  z;

	      We return the indent of "while ( x )".

	      We impose no roof around here because compound
	      statements can be arbitrary long and often are. This is
	      one thing we are obliged to get right in an indenter.
	    */

	    if ( yyBraceDepth > 0 ) {
		do {
		    if ( !readLine() )
			break;
		} while ( yyBraceDepth > 0 );
	    }

	    LinizerState hookState;

	    if ( yyBraceDepth == 0 ) {
		while ( isContinuationLine() )
		    readLine();
		hookState = yyLinizerState;

		readLine();
		if ( yyBraceDepth == 0 ) {
		    do {
			if ( !matchBracelessControlStatement() )
			    break;
			hookState = yyLinizerState;
		    } while ( readLine() );
		}
	    } else {
		hookState = yyLinizerState;
	    }

	    yyLinizerState = hookState;

	    while ( isContinuationLine() )
		readLine();
	    return indentOfLine( yyLine ) - yyBraceDepth * ppIndentSize;
	}

	followedByLeftBrace = ( firstNonWhiteSpace(yyLine) == QChar('{') );

	if ( !readLine() )
	    break;
    }
    return 0;
}

/*
  Returns the recommended indent for the bottom line of yyProgram for
  the cases where indentWhenBottomLineStartsInCComment() won't do.
  It uses indentForContinuationLine() and indentForStandaloneLine(),
  defined above.
*/
static int indentWhenBottomLineStartsInCode()
{
    int indent = indentForContinuationLine();
    if ( indent == NotAContinuationLine )
	indent = indentForStandaloneLine();
    return indent;
}

/*
  Returns the recommended indent for the bottom line of program.
  Unless null, typedIn stores the character of yyProgram that
  triggered reindentation.

  This function works better if typedIn is set properly; it is
  slightly more conservative if typedIn is completely wild, and
  slighly more liberal if typedIn is always null. The user might be
  annoyed by the liberal behavior if she is trying something unusual.
*/
int indentForBottomLine( const QStringList& program, QChar typedIn )
{
    int indent;

    if ( program.isEmpty() )
	return 0;
    yyProgram = program;

    const QString& bottomLine = program.last();
    QChar firstCh = firstNonWhiteSpace( bottomLine );

    if ( bottomLineStartsInCComment() ) {
	/*
	  The last line starts in a C-style comment. Indent it
	  smartly, unless the user has already played around with it,
	  in which case it's better to leave her stuff alone.
	*/
	if ( isOnlyWhiteSpace(bottomLine) ) {
	    indent = indentWhenBottomLineStartsInCComment();
	} else {
	    indent = indentOfLine( bottomLine );
	}
    } else if ( okay(typedIn, QChar('#')) && firstCh == QChar('#') ) {
	/*
	  Preprocessor directives go flush left.
	*/
	indent = 0;
    } else {
	indent = indentWhenBottomLineStartsInCode();

	if ( okay(typedIn, QChar('{')) && firstCh == QChar('{') ) {
	    /*
	      Let's be careful. It's

		  if ( x )
		      y;

	      but

		  if ( x )
		  {

	      Otherwise, left braces behave like anything else.
	    */
	    restartLinizer();
	    if ( matchBracelessControlStatement() )
		indent -= ppIndentSize;
	} else if ( okay(typedIn, QChar('}')) && firstCh == QChar('}') ) {
	    /*
	      A closing brace is one level more to the left than the
	      code it follows.
	    */
	    indent -= ppIndentSize;
	} else if ( okay(typedIn, QChar(':')) &&
		    bottomLine.find(label) == 0 ) {
	    /*
	      Move a case or goto label one level to the left, but
	      only if the user did not play around with it yet. Some
	      users have exotic tastes in the matter, and most users
	      probably are not patient enough to wait for the final
	      ':' to format their code properly.
	    */
	    if ( indentOfLine(bottomLine) == indent )
		indent -= ppIndentSize;
	    else
		indent = indentOfLine( bottomLine );
	}
    }
    yyProgram = QStringList();
    return QMAX( 0, indent );
}

#ifdef Q_TEST_YYINDENT
/*
  Test driver.
*/

#include <qfile.h>
#include <qtextstream.h>

#include <errno.h>

static QString fileContents( const QString& fileName )
{
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "yyindent error: Cannot open file '%s' for reading: %s",
		  fileName.latin1(), strerror(errno) );
	return QString::null;
    }

    QTextStream t( &f );
    QString contents = t.read();
    f.close();
    if ( contents.isEmpty() )
	qWarning( "yyindent error: File '%s' is empty", fileName.latin1() );
    return contents;
}

int main( int argc, char **argv )
{
    if ( argc != 2 ) {
	qWarning( "usage: yyindent file.cpp" );
	return 1;
    }

    QString code = fileContents( QString(argv[1]) );
    QStringList program = QStringList::split( QChar('\n'), code, TRUE );
    QStringList p;
    QString out;

    while ( !program.isEmpty() && program.last().stripWhiteSpace().isEmpty() )
	program.remove( program.fromLast() );

    QStringList::ConstIterator line = program.begin();
    while ( line != program.end() ) {
	p.push_back( *line );
	QChar typedIn = firstNonWhiteSpace( *line );
	if ( p.last().endsWith(QChar(':')) )
	    typedIn = QChar( ':' );
	int indent = indentForBottomLine( p, typedIn );

	if ( !(*line).stripWhiteSpace().isEmpty() ) {
	    for ( int j = 0; j < indent; j++ )
		out += QChar( ' ' );
	    out += (*line).stripWhiteSpace();
	}
	out += QChar( '\n' );
	line++;
    }

    while ( out.endsWith(QChar('\n')) )
	out.truncate( out.length() - 1 );

    printf( "%s\n", out.latin1() );
    return 0;
}
#endif
