#include "qcppsyntaxhighlighter.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

static QString keywords[] = {
    // C++ keywords
    "and",
    "and_eq",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "compl",
    "const",
    "const_cast",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "FALSE",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "true",
    "TRUE",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    // additional "keywords" intoduced by Qt
    "slots",
    "signals",
    "uint",
    "ushort",
    "ulong",
    "emit",
    // end of array
    QString::null
};

static QMap<int, QMap<QString, int > > *wordMap = 0;

QCppSyntaxHighlighter::QCppSyntaxHighlighter( QTextEditDocument *d )
    : QTextEditSyntaxHighlighter( d ), lastFormat( 0 ), lastFormatId( -1 )
{
    createFormats();
    if ( wordMap )
	return;

    wordMap = new QMap<int, QMap<QString, int> >;
    int len;
    for ( int i = 0; keywords[ i ] != QString::null; ++i ) {
	len = keywords[ i ].length();
	if ( !wordMap->contains( len ) )
	    wordMap->insert( len, QMap<QString, int >() );
	QMap<QString, int> &map = wordMap->operator[]( len );
	map[ keywords[ i ] ] = Keyword;
    }
}

void QCppSyntaxHighlighter::createFormats()
{
    addFormat( Standard,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::black ) );
    addFormat( Comment,
	       new QTextEditFormat( QFont( "times", 12, QFont::Bold, TRUE ), Qt::red ) );
    addFormat( Number,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::darkBlue ) );
    addFormat( String,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::darkGreen ) );
    addFormat( Type,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::darkMagenta ) );
    addFormat( Keyword,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::darkYellow ) );
    addFormat( PreProcessor,
	       new QTextEditFormat( QFont( "helvetica", 12, QFont::Bold ), Qt::darkBlue ) );
}

void QCppSyntaxHighlighter::highlighte( QTextEditParag *string, int start, bool invalidate )
{

    QTextEditFormat *formatStandard = format( Standard );
    QTextEditFormat *formatComment = format( Comment );
    QTextEditFormat *formatNumber = format( Number );
    QTextEditFormat *formatString = format( String );
    QTextEditFormat *formatType = format( Type );
    QTextEditFormat *formatPreProcessor = format( PreProcessor );

    QString s = string->string()->toString();

    // states
    const int StateStandard = 0;
    const int StateCommentStart1 = 1;
    const int StateCCommentStart2 = 2;
    const int StateCppCommentStart2 = 3;
    const int StateCComment = 4;
    const int StateCppComment = 5;
    const int StateCCommentEnd1 = 6;
    const int StateCCommentEnd2 = 7;
    const int StateStringStart = 8;
    const int StateString = 9;
    const int StateStringEnd = 10;
    const int StateString2Start = 11;
    const int StateString2 = 12;
    const int StateString2End = 13;
    const int StateNumber = 14;
    const int StatePreProcessor = 15;

    // tokens
    const int InputAlpha = 0;
    const int InputNumber = 1;
    const int InputAsterix = 2;
    const int InputSlash = 3;
    const int InputParen = 4;
    const int InputSpace = 5;
    const int InputHash = 6;
    const int InputQuotation = 7;
    const int InputApostrophe = 8;
    const int InputSep = 9;

    static uchar table[ 16 ][ 10 ] = {
	
	{ StateStandard,  StateNumber, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStandard
	
	{ StateStandard, StateStandard, StateCCommentStart2, StateCppCommentStart2, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCommentStart1
	
	{ StateCComment, StateCComment, StateCCommentEnd1, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment }, // StateCCommentStart2
	
	{ StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment }, // StateCppCommentStart2
	
	{ StateCComment, StateCComment, StateCCommentEnd1, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment }, // StateCComment
	
	{ StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment, StateCppComment }, // StateCppComment
	
	{ StateCComment, StateCComment, StateCCommentEnd1, StateCCommentEnd2, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment, StateCComment }, // StateCCommentEnd1
	
	{ StateStandard, StateNumber, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCCommentEnd2
	
	{ StateString, StateString, StateString, StateString, StateString, StateString, StateString, StateStringEnd, StateString, StateString }, // StateStringStart
	
	{ StateString, StateString, StateString, StateString, StateString, StateString, StateString, StateStringEnd, StateString, StateString }, // StateString
	
	{ StateStandard, StateStandard, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStringEnd
	
	{ StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2End, StateString2 }, // StateString2Start
	
	{ StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2, StateString2End, StateString2 }, // StateString2
	
	{ StateStandard, StateStandard, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateString2End

	{ StateStandard, StateNumber, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateNumber
	
	{ StatePreProcessor, StateStandard, StateStandard, StateCommentStart1, StateStandard, StateStandard, StatePreProcessor, StateStringStart, StateString2Start, StateStandard } // StatePreProcessor
	
    };

    QString buffer;

    int state = StateStandard;
    if ( string->prev() ) {
	if ( string->prev()->endState() == -1 )
	    highlighte( string->prev(), 0, FALSE );
	state = string->prev()->endState();
    }
    int input;
    int i = start;
    bool lastWasBackSlash = FALSE;
    bool makeLastStandard = FALSE;

    string->parenList().clear();

    while ( TRUE ) {
	QChar c = s[ i ];
	
	if ( lastWasBackSlash ) {
	    input = InputSep;
	} else {
	    switch ( c ) {
	    case '*':
		input = InputAsterix;
		break;
	    case '/':
		input = InputSlash;
		break;
	    case '(': case '[': case '{':
		input = InputParen;
		string->parenList() << QTextEditParag::Paren( QTextEditParag::Paren::Open, c, i );
		break;
	    case ')': case ']': case '}':
		input = InputParen;
		string->parenList() << QTextEditParag::Paren( QTextEditParag::Paren::Closed, c, i );
		break;
	    case '#':
		input = InputHash;
		break;
	    case '"':
		input = InputQuotation;
		break;
	    case '\'':
		input = InputApostrophe;
		break;
	    case ' ':
		input = InputSpace;
		break;
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9': case '0':
		//input = InputNumber;
		input = InputAlpha;
		break;
	    default: {
		    if ( c.isLetter() || c == '_' )
			input = InputAlpha;
		    else
			input = InputSep;
		} break;
	    }
	}
	
	lastWasBackSlash = !lastWasBackSlash && c == '\\';
	
	if ( input == InputAlpha )
	    buffer += c;
	
    	state = table[ state ][ input ];

	switch ( state ) {
	case StateStandard: {
	    int len = buffer.length();
	    string->setFormat( i, 1, formatStandard, FALSE );
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    if ( buffer.length() > 0 && input != InputAlpha ) {
		if ( buffer[ 0 ] == 'Q' ) {
		    string->setFormat( i - buffer.length(), buffer.length(), formatType, FALSE );
		} else {
		    QMap<int, QMap<QString, int > >::Iterator it = wordMap->find( len );
		    if ( it != wordMap->end() ) {
			QMap<QString, int >::Iterator it2 = ( *it ).find( buffer );
			if ( it2 != ( *it ).end() )
			    string->setFormat( i - buffer.length(), buffer.length(), format( ( *it2 ) ), FALSE );
		    }
		}
		buffer = QString::null;
	    }
	} break;
	case StateCommentStart1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = TRUE;
	    buffer = QString::null;
	    break;
	case StateCCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = QString::null;
	    break;
	case StateCppCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = QString::null;
	    break;
	case StateCComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCppComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateStringStart:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = QString::null;
	    break;
	case StateStringEnd:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2Start:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2End:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateNumber:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatNumber, FALSE );
	    buffer = QString::null;
	    break;
	case StatePreProcessor:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatPreProcessor, FALSE );
	    buffer = QString::null;
	    break;
	}

	i++;
	if ( i >= (int)s.length() )
	    break;
    }

    if ( state == StateCComment ||
	 state == StateCCommentEnd1 ) {
	string->setEndState( StateCComment );
    } else if ( state == StateString ) {
	string->setEndState( StateString );
    } else if ( state == StateString2 ) {
	string->setEndState( StateString2 );
    } else {
	string->setEndState( StateStandard );
    }

    string->setFirstHighlighte( FALSE );

    if ( invalidate && string->next() &&
	 !string->next()->firstHighlighte() && string->next()->endState() != -1 ) {
	QTextEditParag *p = string->next();
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

QTextEditFormat *QCppSyntaxHighlighter::format( int id )
{
    if ( lastFormatId == id  && lastFormat )
	return lastFormat;

    QTextEditFormat *f = formats[ id ];
    lastFormat = f ? f : formats[ 0 ];
    lastFormatId = id;
    return lastFormat;
}

void QCppSyntaxHighlighter::addFormat( int id, QTextEditFormat *f )
{
    formats.insert( id, f );
}

void QCppSyntaxHighlighter::removeFormat( int id )
{
    formats.remove( id );
}

QCppIndent::QCppIndent( QTextEditDocument *d )
    : QTextEditIndent( d )
{
}

void QCppIndent::indent( QTextEditParag *parag, int *oldIndent, int *newIndent )
{
    // ####################
    // This is a very simple (and too simple for a good programming editor)
    // implementation of auto-indentation (works like in all simple editors
    // like kwrite - ok a bit better but still for from working well!)
    // So, auto-indentation has to be rewritten to work depending on open/closed
    // parens like e.g. in (x)emacs!
    // ####################

    int oi = 0;
    int i = 0;
    while ( i < parag->string()->length() - 1 ) {
	if ( parag->string()->at( i ).c == ' ' || parag->string()->at( i ).c == '\t' )
	    ++oi;
	else
	    break;
	++i;
    }

    if ( oldIndent )
	*oldIndent = oi;

    int lastIndent = 0;
    QString indentString;
    QTextEditParag *p = parag->prev();
    if ( p ) {
	i = 0;
	while ( i < (int)p->string()->length() - 1 ) {
	    if ( p->string()->at( i ).c == ' ' || p->string()->at( i ).c == '\t' ) {
		++lastIndent;
		indentString += p->string()->at( i ).c;
	    } else {
		break;
	    }
	    ++i;
	}
    }


    QString s( parag->string()->toString() );
    s = s.simplifyWhiteSpace();
    int diff = 0;
    if ( /*s.left( 1 ) == "{" || */ s.left( 1 ) == "}" )
	diff--;
    if ( p ) {
	s = p->string()->toString();
	s = s.simplifyWhiteSpace();
	if ( s.right( 1 ) == "{" )
	    diff++;
    }

    if ( diff == 1 ) {
	lastIndent++;
	indentString += '\t';
    } else if ( diff == -1 && indentString.length() ) {
	lastIndent--;
	indentString.remove( indentString.length() - 1, 1 );
    }
	

    if ( newIndent )
	*newIndent = lastIndent;

    if ( oi > 0 )
	parag->remove( 0, oi );
    if ( !indentString.isEmpty() )
	parag->insert( 0, indentString );

    if ( parag->string()->length() == 0 )
 	parag->append( " " );
}
