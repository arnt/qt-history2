#include "indent_cpp.h"
#include "qregexp.h"

Indent_CPP::Indent_CPP()
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

void Indent_CPP::indent( QTextDocument *, QTextParag *parag, int *oldIndent, int *newIndent )
{
    int oi = 0, lastIndent = 0;
    int braceLevel = 0, parenLevel = 0;
    int delta = 0;
    bool seenSemicolon = FALSE, seenBraces = FALSE;
    bool caseStat = FALSE;
    QString s;
    const QTextParag *p = parag;
    while ( p ) {
	delta = 0;
	lastIndent = 0;
	int i = 0;
	s = p->string()->toString();
	// determine size of indentation
	while ( i < (int)s.length() - 1 ) {
	    QChar c = s.at( i );
	    if ( c == ' ' )
		lastIndent++;
	    else if ( c == '\t' )
		lastIndent += 8;
	    else
		break;
	    ++i;
	}
	// return no of indentation characters of current line
	if ( p == parag ) {
	    oi = i;
	    if ( oldIndent )
		*oldIndent = i;
	}

	// parse line for braces and semicolons
	uncomment( s );
	s.replace( QRegExp( "\\s" ), "" );	// remove spaces
	if ( p == parag && s.startsWith( "case" ) )
	    caseStat = TRUE;
	if ( p == parag )			// current line ? 1st char only
	    s.truncate( 1 );
	i = s.length()-1;
	while ( i >= 0 ) {
	    QChar c = s[ i ];
	    if ( c == ";" && braceLevel == 0 && parenLevel == 0 ) {
		seenSemicolon = TRUE;
	    } else if ( c == "}" ) {
		braceLevel++;
		seenBraces = TRUE;
	    } else if ( c == "{" ) {
		braceLevel--;
	    } else if ( c == ")" ) {
		parenLevel++;
	    } else if ( c == "(" ) {
		parenLevel--;
	    }
	    i--;
	}
	i++;
	s = s.mid( i );
	// any indication for an increased indentation ?
	if ( braceLevel < 0 ||
	     braceLevel == 0 && !seenBraces && !seenSemicolon &&
	     ( s.startsWith( "if(" ) || s.startsWith( "elseif(" ) ||
	       s.startsWith( "do(" ) || s.startsWith( "switch(" ) ||
	       s.startsWith( "for(" ) || s.startsWith( "while(" ) ) ) {
	    delta = 4;
	    break;
	} else if ( braceLevel == 1 && !seenSemicolon &&
		    ( s == "}else" || s.startsWith( "}while(" ))) {
	    delta = 4;
	    break;
	} else if ( braceLevel == 0 && !seenSemicolon && s == "else" ) {
	    delta = 4;
	    break;
	} else if ( braceLevel == 0 && s.startsWith( "case" ) && !caseStat ) {
	    delta = 4;
	    break;
	}
	// can't decide yet. move up one line.
	p = p->prev();
    }

    if ( p == parag )
	delta = 0;
    QString indentString;
    indentString.fill( ' ', lastIndent+delta );
    indentString.append( "a" );
    tabify( indentString );
    indentString.remove( indentString.length() - 1, 1 );
    lastIndent = indentString.length();

    if ( newIndent )
	*newIndent = lastIndent;

    if ( oi > 0 )
	parag->remove( 0, oi );
    if ( !indentString.isEmpty() )
	parag->insert( 0, indentString );

    if ( parag->string()->length() == 0 )
 	parag->append( " " );
}

void Indent_CPP::tabify( QString &s )
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

void Indent_CPP::untabify( QString &s )
{
    s.replace( QRegExp( "\t" ), "        " );
}
