/*
  cpptoqsconverter.cpp
*/

#include <qregexp.h>

#include "config.h"
#include "cpptoqsconverter.h"

#define CONFIG_QUICK                    "quick"
#define CONFIG_INDENTSIZE               "indentsize"

void setTabSize( int size );
void setIndentSize( int size );
int columnForIndex( const QString& t, int index );
int indentForBottomLine( const QStringList& program, QChar typedIn );

static QString balancedParens = "(?:[^()]+|\\([^()]*\\))*";

int CppToQsConverter::tabSize;

CppToQsConverter::CppToQsConverter()
{
    clearState();
}

ClassNode *CppToQsConverter::findClassNode( Tree *qsTree,
					    const QString& qtName )
{
    ClassNode *classe = (ClassNode *) qsTree->findNode( qtName, Node::Class );
    if ( classe == 0 )
	classe = (ClassNode *) qsTree->findNode( qtName.mid(1), Node::Class );
    return classe;
}

QString CppToQsConverter::convertedDataType( Tree *qsTree,
					     const QString& leftType,
					     const QString& /* rightType */ )
{
    QString s = leftType;

    if ( s.startsWith("const ") )
	s = s.mid( 6 );
    while ( s.endsWith("*") || s.endsWith("&") || s.endsWith(" ") )
	s.truncate( s.length() - 1 );

    switch ( s[0].unicode() ) {
    case 'Q':
	if ( s == "QCString" ) {
	    return "String";
	} else {
	    Node *node = findClassNode( qsTree, s );
	    if ( node == 0 ) {
		return "";
	    } else {
		return node->name();
	    }
	}
	break;
    case 'b':
	if ( s == "bool" )
	    return "Boolean";
	break;
    case 'c':
	if ( s == "char" ) {
	    if ( leftType == "const char *" ) {
		return "String";
	    } else {
		return "Number";	    
	    }
	}
	break;
    case 'd':
	if ( s == "double" )
	    return "Number";
	break;
    case 'f':
	if ( s == "float" )
	    return "Number";
    case 'i':
	if ( s == "int" )
	    return "Number";
	break;
    case 'l':
	if ( s == "long" || s == "long int" || s == "long long" ||
	     s == "long long int" || s == "long double" )
	    return "Number";
	break;
    case 's':
	if ( s == "short" || s == "short int" || s == "signed char" ||
	     s == "signed short" || s == "signed short int" || s == "signed" ||
	     s == "signed int" || s == "signed long" || s == "signed long int" )
	    return "Number";
	break;
    case 'u':
	if ( s == "uchar" || s == "unsigned" || s == "unsigned char" ||
	     s == "ushort" || s == "unsigned short" ||
	     s == "unsigned short int" || s == "uint" || s == "unsigned int" ||
	     s == "ulong" || s == "unsigned long" || s == "unsigned long int" )
	    return "Number";
	break;
    case 'v':
	if ( s == "void" )
	    return "";
    }
    return s;
}

QString CppToQsConverter::convertedCode( Tree *qsTree, const QString& code )
{
    QString result;
    QStringList program;
    QStringList comments;
    int leftSizeWidth = 0;

    clearState();

    QStringList originalLines = QStringList::split( "\n", code, TRUE );
    QStringList::ConstIterator ol = originalLines.begin();
    while ( ol != originalLines.end() ) {
	QString code = *ol;
	QString comment;

	int slashSlash = code.find( "//" );
	if ( slashSlash != -1 ) {
	    comment = code.mid( slashSlash );
	    code.truncate( slashSlash );
	    
	    if ( !code.stripWhiteSpace().isEmpty() ) {
		while ( code[code.length() - 1].isSpace() )
		    code.truncate( code.length() - 1 );
	    }
	}

	code = convertCodeLine( qsTree, program, code );
	program.append( code );
	comments.append( comment );

	int n = indentForBottomLine( program, QChar::null );
	for ( int i = 0; i < n; i++ )
	    program.last().prepend( " " );

	int width = columnForIndex( code, code.length() );
	if ( width > leftSizeWidth )
	    leftSizeWidth = width;
	++ol;
    }

    leftSizeWidth += 2;
    leftSizeWidth = ( (leftSizeWidth + (tabSize - 1)) / tabSize ) * tabSize;

    QStringList::ConstIterator p = program.begin();
    QStringList::ConstIterator c = comments.begin();
    while ( c != comments.end() ) {
	if ( c != comments.begin() )
	    result += "\n";
	result += *p;
	if ( !(*c).isEmpty() ) {
	    if ( !(*p).stripWhiteSpace().isEmpty() ) {
		int i = columnForIndex( *p, (*p).length() );
		while ( i++ < leftSizeWidth )
		    result += " ";
	    }
	    result += *c;
	}
	++p;
	++c;
    }
    return result;
}

void CppToQsConverter::initialize( const Config& config )
{
    tabSize = config.getInt( CONFIG_TABSIZE );
    setTabSize( tabSize );

    int size = config.getInt( CONFIG_QUICK + Config::dot + CONFIG_INDENTSIZE );
    if ( size > 0 )
	setIndentSize( size );
}

void CppToQsConverter::terminate()
{
}

void CppToQsConverter::clearState()
{
    returnType = "";
}

QString CppToQsConverter::convertCodeLine( Tree *qsTree,
					   const QStringList& program,
					   QString code )
{
    static QString dataTypeFmt = "[^=()]*\\s+[*&]?";
    static QRegExp funcPrototypeRegExp(
	"(" + dataTypeFmt + ")\\s+[*&]?([A-Z][a-zA-Z_0-9]*::)"
	"([a-z][a-zA-Z_0-9]*)\\(([^);]*)(\\)?)(?:\\s*const)?" );
    static QRegExp paramRegExp(
	"\\s*(" + dataTypeFmt + ")([a-z][a-zA-Z_0-9]*)\\s*(,)?\\s*" );

    int start = 0;
    while ( start < (int) code.length() && code[start].isSpace() )
	start++;
    code = code.mid( start );
    if ( code.isEmpty() || code == "{" || code == "}" )
	return code;

    QString result;

    if ( funcPrototypeRegExp.exactMatch(code) ) {
	returnType = funcPrototypeRegExp.cap( 1 );
	QString className = funcPrototypeRegExp.cap( 2 );
	QString funcName = funcPrototypeRegExp.cap( 3 );
	QString params = funcPrototypeRegExp.cap( 4 );
	bool toBeContinued = funcPrototypeRegExp.cap( 5 ).isEmpty();

	className.replace( "::", "." );

	result = "function " + className + funcName + "(";

	if ( !toBeContinued ) {
	    result += ")";
	    returnType = convertedDataType( qsTree, returnType );
	    if ( !returnType.isEmpty() )
		result += " : " + returnType;
	}
    } else {
	result = code;
    }
    return result;
}
