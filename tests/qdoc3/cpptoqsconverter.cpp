/*
  cpptoqsconverter.cpp
*/

#include <qregexp.h>

#include "config.h"
#include "cpptoqsconverter.h"

static QString balancedParens = "(?:[^()]+|\\([^()]*\\))*";

int CppToQsConverter::tabSize;

int CppToQsConverter::columnForIndex( const QString& str, int index )
{
    int endOfPrevLine = str.findRev( "\n", index - 1 );
    int column = 0;

    for ( int i = endOfPrevLine + 1; i < index; i++ ) {
	if ( str[i] == '\t' ) {
	    column = ( (column / tabSize) + 1 ) * tabSize;
	} else {
	    column++;
	}
    }
    return column;
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
    QStringList leftSide;
    QStringList rightSide;
    int leftSizeWidth = 0;

    indent = 0;
    braceDepth = 0;
    parenDepth = 0;

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

	    int width = columnForIndex( code, code.length() );
	    if ( width > leftSizeWidth )
		leftSizeWidth = width;
	}
	convertCodeLine( qsTree, code );
	updateDelimDepths( code );
	leftSide.append( code );
	rightSide.append( comment );
	++ol;
    }

    leftSizeWidth += 2;
    leftSizeWidth = ( (leftSizeWidth + (tabSize - 1)) / tabSize ) * tabSize;

    QStringList::ConstIterator ls = leftSide.begin();
    QStringList::ConstIterator rs = rightSide.begin();
    while ( rs != rightSide.end() ) {
	if ( rs != rightSide.begin() )
	    result += "\n";
	result += *ls;
	if ( !(*rs).isEmpty() ) {
	    if ( !(*ls).stripWhiteSpace().isEmpty() ) {
		int i = columnForIndex( *ls, (*ls).length() );
		while ( i++ < leftSizeWidth )
		    result += " ";
	    }
	    result += *rs;
	}
	++ls;
	++rs;
    }
    return result;
}

void CppToQsConverter::initialize( const Config& config )
{
    tabSize = config.getInt( CONFIG_TABSIZE );
}

void CppToQsConverter::terminate()
{
}

int CppToQsConverter::convertCodeLine( Tree *qsTree, QString& code )
{
    static QString dataTypeFmt = "[^=()]*\\s+[*&]?";
    static QRegExp funcPrototypeRegExp(
	"(" + dataTypeFmt + ")\\s+[*&]?([A-Z][a-zA-Z_0-9]*::)"
	"([a-z][a-zA-Z_0-9]*)\\(([^);]*)\\)?(?:\\s*const)?" );
    static QRegExp paramRegExp(
	"\\s*(" + dataTypeFmt + ")([a-z][a-zA-Z_0-9]*)\\s*(,)?\\s*" );

    int start = 0;
    while ( start < (int) code.length() && code[start].isSpace() )
	start++;
    QString core = code.mid( start );
    if ( core.isEmpty() || core == "{" || core == "}" )
	return indent;

    if ( braceDepth == 0 && parenDepth == 0 &&
	 funcPrototypeRegExp.exactMatch(core) ) {
	QString returnType = funcPrototypeRegExp.cap( 1 );
	QString className = funcPrototypeRegExp.cap( 2 );
	QString funcName = funcPrototypeRegExp.cap( 3 );
	QString params = funcPrototypeRegExp.cap( 4 );
	bool continued = funcPrototypeRegExp.cap( 5 ).isEmpty();

	className.replace( "::", "." );

	core = "function " + className + funcName + "(";


	if ( continued ) {
	    core += ")";
	    returnType = convertedDataType( qsTree, returnType );
	    if ( !returnType.isEmpty() )
		core += " : " + returnType;
	}
    } else {
	// ...
    }
    code.replace( start, code.length() - start, core );
    return indent;
}

void CppToQsConverter::updateDelimDepths( const QString& code )
{
    QChar quote( 'X' );

    for ( int i = 0; i < (int) code.length(); i++ ) {
	if ( quote == 'X' ) {
	    if ( code[i] == QChar('"') || code[i] == QChar('\'') )
		quote = code[i];
	} else if ( code[i] == quote ) {
	    quote = 'X';
	} else if ( code[i] == QChar('{') ) {
	    braceDepth++;
	} else if ( code[i] == QChar('}') ) {
	    braceDepth--;
	} else if ( code[i] == QChar('(') ) {
	    parenDepth++;
	} else if ( code[i] == QChar(')') ) {
	    parenDepth--;
	}
    }
}
