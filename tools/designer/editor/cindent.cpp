#include "cindent.h"
#include "qregexp.h"

extern int indentForBottomLine( const QStringList& program, QChar typedIn );
extern void setTabSize( int s );
extern void setIndentSize( int s );

CIndent::CIndent()
    : QTextIndent(), tabSize( 8 ), indentSize( 4 ),
      autoIndent( TRUE ), keepTabs( TRUE ), lastDoc( 0 )
{
}

static int indentation( const QString &s )
{
    if ( s.simplifyWhiteSpace().length() == 0 )
	return 0;
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

void CIndent::tabify( QString &s )
{
    if ( !keepTabs )
	return;
    int i = 0;
    for ( ;; ) {
	for ( int j = i; j < (int)s.length(); ++j ) {
	    if ( s[ j ] != ' ' && s[ j ] != '\t' ) {
		if ( j > i ) {
		    QString t  = s.mid( i, j - i );
		    int spaces = 0;
		    for ( int k = 0; k < (int)t.length(); ++k )
			spaces += ( t[ k ] == ' ' ? 1 : tabSize );
		    s.remove( i, t.length() );
		    int tabs = spaces / tabSize;
		    spaces = spaces - ( tabSize * tabs );
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

void CIndent::indentLine( QTextParag *p, int &oldIndent, int &newIndent )
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


void CIndent::indent( QTextDocument *doc, QTextParag *p, int *oldIndent, int *newIndent )
{
    lastDoc = doc;
    int oi = indentation( p->string()->toString() );
    QStringList code;
    QTextParag *parag = doc->firstParag();
    while ( parag ) {
	code << parag->string()->toString();
	if ( parag == p )
	    break;
	parag = parag->next();
    }

    int ind = indentForBottomLine( code, QChar::null );
    ind = ( ind / 4 ) * indentSize;
    indentLine( p, oi, ind );
    if ( oldIndent )
	*oldIndent = oi;
    if ( newIndent )
	*newIndent = ind;
}

void CIndent::reindent()
{
    if ( !lastDoc )
	return;
    // #### this is sloooooooow (O(n^2))
    QTextParag *parag = lastDoc->firstParag();
    while ( parag ) {
	indent( lastDoc, parag, 0, 0 );
	parag = parag->next();
    }
}

void CIndent::setTabSize( int ts )
{
    tabSize = ts;
    ::setTabSize( ts );
    reindent();
}

void CIndent::setIndentSize( int is )
{
    indentSize = is;
    ::setIndentSize( is );
    reindent();
}
