#include "cindent.h"
#include "qregexp.h"

CIndent::CIndent()
    : QTextIndent()
{
}

static void uncomment( QString &s )
{
    int i;
    if ( ( i = s.find( "//" ) ) != -1 )
	s = s.remove( i, s.length() - i + 1 );
    if ( ( i = s.findRev( "/*" ) ) != -1 ) {
	int j = s.find( "*/", i + 1 );
	if ( j == -1 || j == (int)s.length() - 2 )
	    s = s.remove( i, s.length() - i + 1 );
    }
}

void CIndent::tabify( QString &s )
{
    int i = 0;
    while ( TRUE ) {
	for ( int j = i; j < (int)s.length(); ++j ) {
	    if ( s[ j ] != ' ' && s[ j ] != '\t' ) {
		if ( j > i ) {
		    QString t  = s.mid( i, j - i );
		    int spaces = 0;
		    for ( int k = 0; k < (int)t.length(); ++k )
			spaces += ( t[ k ] == ' ' ? 1 : 8 );
		    s.remove( i, t.length() );
		    int tabs = spaces / 8;
		    spaces = spaces - ( 8 * tabs );
		    QString tmp;
		    tmp.fill( ' ', spaces );
		    if ( spaces > 0 )
			s.insert( i, tmp );
		    tmp.fill( '\t', tabs );
		    if ( tabs > 0 )
			s.insert( i, tmp );
		}
		break;
	    }
	}
	i = s.find( '\n', i );
	if ( i == -1 )
	    break;
	++i;
    }
}

void CIndent::untabify( QString &s )
{
    s.replace( QRegExp( "\t" ), "        " );
}

int CIndent::indentation( const QString &s ) const
{
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
	QChar c = s.at( i );
	if ( c == ' ' )
	    ind++;
	else if ( c == '\t' )
	    ind += 8;
	else
	    break;
	++i;
    }
    return ind;
}

void CIndent::simplifyLine( QString &s ) const
{
    uncomment( s );
    s.replace( QRegExp( "\\s" ), "" ); // remove spaces
}

void CIndent::indentLine( QTextParag *p, int &oldIndent, int &newIndent ) const
{
    QString indentString;
    indentString.fill( ' ', newIndent );
    indentString.append( "a" );
    tabify( indentString );
    indentString.remove( indentString.length() - 1, 1 );
    newIndent = indentString.length();
    oldIndent = 0;
    while ( p->length() > 0 && ( p->at( 0 )->c == ' ' || p->at( 0 )->c == '\t' ) ) {
	++oldIndent;
	p->remove( 0, 1 );
    }
    if ( p->string()->length() == 0 )
 	p->append( " " );
    if ( !indentString.isEmpty() )
	p->insert( 0, indentString );
}

static bool cppFunctionStart( const QString &s )
{
    // try to find a C++ function start
    // is there a way to do it really reliable?
    int cc = s.find( "::" );
    if ( cc == -1 )
	return FALSE;
    int op = s.contains( '(' );
    if ( op != 1 )
	return FALSE;
    int cp = s.contains( ')' );
    if ( cp != 1 )
	return FALSE;
    if ( s.find( '(' ) > s.find( ')' ) )
	return FALSE;
    if ( s.find( ';' ) != -1 )
	return FALSE;
    return TRUE;
}

bool CIndent::isBlockStart( const QString &s ) const
{
    bool b = s.startsWith( "if(" ) ||
	     s.startsWith( "else" ) || // also covers else if
	     s.startsWith( "do(" ) ||
	     s.startsWith( "switch(" ) ||
	     s.startsWith( "for(" ) ||
	     s.startsWith( "while(" ) ||
	     s.startsWith( "with(" ) ||
	     s.startsWith( "function" ) ||
	     s.startsWith( "class" ) ||
	     s.startsWith( "}else" ); // also covers else if
    if ( b )
	return TRUE;
    return cppFunctionStart( s );
}

void CIndent::findBlockStart( QTextParag **p, QString &s, int &i, const QChar &c, const QChar &opposite )
{
    int count = -1;
    --i;
    while ( *p ) {
	for ( ; i >= 0; --i ) {
	    if ( s[i] == opposite )
		--count;
	    else if ( s[i] == c )
		++count;
	    if ( count == 0 )
		return;
	}
	if ( (*p)->prev() ) {
	    *p = (*p)->prev();
	    s = (*p)->string()->toString();
	    simplifyLine( s );
	    i = s.length() - 1;
	} else {
	    i = 0;
	    break;
	}
    }
}

void CIndent::indentParenBlock( QTextParag **p, int &currIndent, int &i )
{
    ++i;
    int line = 0;
    QTextParag *old;
    int incNext = 0;
    while ( p ) {
	QString s = (*p)->string()->toString();
	simplifyLine( s );
	for ( ; i < (int)s.length(); ++i ) {
	    QChar c = s[i];
	    switch ( c ) {
	    case ')':
		if ( curClose.top() != c )
		    break;
		curClose.pop();
		currIndent += incNext;
		return;
	    case ']':
		if ( curClose.top() != c )
		    break;
		curClose.pop();
		currIndent += incNext;
		return;
	    case '(':
	    case '[':
		curClose.push( c == '[' ? ']' : ')' );
		old = *p;
		if ( line == 0 ) {
		    currIndent += 4;
		    if ( startParag )
			extraIndent += 4;
		}
		indentParenBlock( p, currIndent, i );
		if ( old != *p || line == 0 ) {
		    incNext = -4;
		    s = (*p)->string()->toString();
		    simplifyLine( s );
		}
		break;
	    }
	}
	if ( *p == parag )
	    return;
	*p = (*p)->next();
	i = 0;
	if ( line == 0 )
	    currIndent += 4;
	currIndent += incNext;
	incNext = 0;
	line++;
    }
}

void CIndent::indentBlock( QTextParag **p, int &currIndent )
{
    int line = 0;
    bool needClosingBrace = FALSE;
    int incNext = 0;
    bool startsUnknownBlock = FALSE;
    while ( p ) {
	QString s = (*p)->string()->toString();
	simplifyLine( s );
	bool blockStart = isBlockStart( s );
	for ( int i = 0; i < (int)s.length(); ++i ) {
	    QChar c = s[i];
	    switch ( c ) {
	    case '{':
		if ( i > 0 && line == 0 ) { // our block
		    incNext += 4;
		    needClosingBrace = TRUE;
		    goto out;
		} else if ( i > 0 && line > 0 ) { // a new block starts
		    goto out;
		} else if ( i == 0 && line == 1 ) { // our block, next line
		    incNext = 4;
		    needClosingBrace = TRUE;
		    goto out;
		} else if ( i == 0 && line > 1 ) { // a scope block
		    startsUnknownBlock = TRUE;
		    goto out;
		} else if ( i == 0 && line == 0 ) { // our scope block
		    incNext = 4;
		    needClosingBrace = TRUE;
		    goto out;
		}
		break;
	    case '}':
		if ( line == 0 || !isBlockStart( s ) ) {
		    currIndent -= 4;
		    if ( isBlockStart( s ) ) {
			needClosingBrace = TRUE;
			incNext = 4;
			goto out;
		    }
		} else {
		    goto out;
		}
		return;	
		break;
	    case '(':
	    case '[':
		if ( !blockStart || line == 0 ) {
		    startParag = *p;
		    extraIndent = 0;
		    int oldIndent = currIndent;
		    QTextParag *old = *p;
		    curClose.push( c == '[' ? ']' : ')' );
		    indentParenBlock( p, currIndent, i );
		    if ( old != *p ) {
			incNext -= 4;
			s = (*p)->string()->toString();
			simplifyLine( s );
		    } else if ( *p == startParag ) {
			currIndent = oldIndent;
		    }
		}
		break;
	    }
	}
    out:
 	if ( line == 1 && !needClosingBrace )
 	    currIndent += 4;
	if ( line > 0 && ( isBlockStart( s ) || startsUnknownBlock ) ) {
	    QTextParag *old = *p;
	    indentBlock( p, currIndent );
	    if ( old != *p ) {
		s = (*p)->string()->toString();
		simplifyLine( s );
	    }
	    startsUnknownBlock = FALSE;
	    incNext = 0;
	}		
	if ( line == 2 && !needClosingBrace ) {
	    currIndent -= 4;
	    return;
	}
	if ( *p == parag ) {
	    if ( !caseDone && s.find( "case" ) != -1 )
		currIndent -= 4;
	    caseDone = TRUE;
	    return;
	}
	if ( line == 1 && !needClosingBrace ) {
	    currIndent -= 4;
	    return;
	}
	*p = (*p)->next();
	++line;
	currIndent += incNext;
	incNext = 0;
    }
		
}

void CIndent::indent( QTextDocument *doc, QTextParag *p, int *oldIndent, int *newIndent )
{
    curClose.clear();
    caseDone = FALSE;
    QString s = p->string()->toString();
    int oi = indentation( s );
    if ( !p->prev() ) {
	int ni = 0;
	indentLine( p, oi, ni );
	if ( oldIndent )
	    *oldIndent = oi;
	if ( newIndent )
	    *newIndent = ni;
	return;
    }

    parag = p;
#if 0
    int skipOne = 0;
    while ( p ) {
	QString s = p->string()->toString();
	simplifyLine( s );
	for ( int i = s.length() - 1; i >= 0; --i ) {
	    QChar c = s[i];
	    switch ( c ) {
	    case '}':
		findBlockStart( &p, s, i, '{', c );
		skipOne++;
		break;
	    case ']':
		findBlockStart( &p, s, i, '[', c );
		break;
	    case ')':
		findBlockStart( &p, s, i, '(', c );
		break;
	    }
	}
	if ( p != parag && isBlockStart( s ) ) {
	    if ( skipOne == 0 )
		break;
	    else
		skipOne--;
	}
	p = p->prev();
    }
	
    if ( !p )
	p = doc->firstParag();
#else
    Q_UNUSED( doc )
    p = p->prev();
    while ( p ) {
	QString s = p->string()->toString();
	int ind = indentation( s );
	simplifyLine( s );
	if ( ( s.startsWith( "function" ) || s.startsWith( "class" ) || cppFunctionStart( s ) ) && ind == 0 )
	    break;
	p = p->prev();
    }
    if ( !p )
	p = doc->firstParag();
#endif

    s = p->string()->toString();
    int currIndent = indentation( s );

    while ( p ) {
	s = p->string()->toString();
	simplifyLine( s );
	if ( isBlockStart( s ) )
	    indentBlock( &p, currIndent );
	if ( p == parag )
	    break;
	p = p->next();
    }

    indentLine( p, oi, currIndent );
    if ( oldIndent )
	*oldIndent = oi;
    if ( newIndent )
	*newIndent = currIndent;
}

