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
	    Node *node = qsTree->findNode( s, Node::Class );
	    if ( node == 0 )
		node = qsTree->findNode( s.mid(1), Node::Class );

	    if ( node == 0 ) {
		return "UNKNOWN";
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
    QRegExp funcRegExp(
	    "^([ \t]*)(?:[\\w<>,&*]+[ \t]+)+((?:\\w+::)*\\w+)\\((" +
	    balancedParens + ")\\)(?:[ \t]*const)?(?=[ \n\t]*\\{)" );
    QRegExp paramRegExp(
	    "\\s*(const\\s+)?([^\\s=]+)\\b[^=]*\\W(\\w+)\\s*(?:=.*)?" );
    QRegExp qtVarRegExp(
	    "^([ \t]*)(const[ \t]+)?Q([A-Z][A-Za-z_0-9]*)\\s+"
	    "([a-z][A-Za-z_0-9]*)\\s*(?:\\((" + balancedParens +
	    ")\\))?\\s*;" );
    QRegExp signalOrSlotRegExp(
	     "^(SIGNAL|SLOT)\\((" + balancedParens + ")\\)" );
    QString result;
    bool inString = FALSE;
    QChar quote;
    int i = 0;

    while ( i < (int) code.length() ) {
	if ( inString ) {
	    if ( code[i] == quote ) {
		result += code[i++];
		inString = FALSE;
	    } else if ( code[i] == '\\' ) {
		result += code[i++];
		result += code[i++];
	    } else {
		result += code[i++];
	    }
	} else {
	    bool newLine = ( i == 0 || code[i - 1] == '\n' );

	    if ( newLine && funcRegExp.search(code.mid(i)) != -1 ) {
		QString indent = funcRegExp.cap( 1 );
		QString name = funcRegExp.cap( 2 );
		// ### remove QRegExp cast once Mark has Qt 3.1 up and running
		name.replace( QRegExp("::"), "." );
		QStringList params =
			QStringList::split( ",", funcRegExp.cap(3) );
		QStringList::Iterator p = params.begin();
		while ( p != params.end() ) {
		    if ( paramRegExp.exactMatch(*p) ) {
#if 0
			bool isVar = paramRegExp.cap( 1 ).isEmpty();
#endif
			QString qtDataType = paramRegExp.cap( 2 );
			QString name = paramRegExp.cap( 3 );
			*p = "var " + name + " : " +
			     convertedDataType( qsTree, qtDataType );
		    }
		    ++p;
		}
		result += indent + "function " + name + "( " +
			  params.join(", ") + " )";
		i += funcRegExp.matchedLength();
	    } else if ( newLine && qtVarRegExp.search(code.mid(i)) != -1 ) {
		QString indent = qtVarRegExp.cap( 1 );
		bool isVar = qtVarRegExp.cap( 2 ).isEmpty();
		QString quickClass = qtVarRegExp.cap( 3 );
		QString name = qtVarRegExp.cap( 4 );
		QString initializer = qtVarRegExp.cap( 5 ).simplifyWhiteSpace();
		result += indent + ( isVar ? "var " : "const " ) + name;
		if ( initializer.isEmpty() ) {
		    result += " = new " + quickClass + ";";
		} else {
		    result += " = new " + quickClass + "( " + initializer +
			      " );";
		}
		i += qtVarRegExp.matchedLength();
	    } else if ( code[i] == '"' || code[i] == '\'' ) {
		quote = code[i];
		result += code[i++];
		inString = TRUE;
	    } else if ( code[i] == 'F' && code.mid(i, 5) == "FALSE" ) {
		result += "false";
		i += 5;
	    } else if ( code[i] == 'Q' ) {
		if ( code.mid(i - 4, 3) == "new" ) {
		    if ( code[i + 1].lower() == code[i + 1] ) {
			result += code[i++];
		    } else {
			i++;
		    }
		} else {
		    result += "var ";
		    while ( i < (int) code.length() &&
			    code[i].isLetterOrNumber() )
			i++;
		    while ( i < (int) code.length() &&
			    !code[i].isLetterOrNumber() && code[i] != '\n' )
			i++;
		}
	    } else if ( code[i] == 'S' &&
			(code.mid(i, 7) == "SIGNAL(" ||
			 code.mid(i, 5) == "SLOT(") ) {
		if ( signalOrSlotRegExp.search(code, i
#if QT_VERSION >= 0x030100
						      , QRegExp::CaretAtOffset
#endif
					 ) == i ) {
		    result += signalOrSlotRegExp.cap( 1 ) + "(\"" +
			      signalOrSlotRegExp.cap( 2 ) + "\")";
		    i += signalOrSlotRegExp.matchedLength();
		}
	    } else if ( code[i] == 'T' && code.mid(i, 4) == "TRUE" ) {
		result += "true";
		i += 4;
	    } else if ( code[i] == '/' && code[i + 1] == '/' ) {
		int numSpaces = columnForIndex( code, i ) -
				columnForIndex( result, result.length() );
		if ( numSpaces > 0 ) {
		    while ( numSpaces-- > 0 )
			result += " ";
		} else if ( numSpaces < 0 ) {
		    while ( numSpaces++ < 0 && result.right(1) == " " )
			result.truncate( result.length() - 1 );
		}
		while ( i < (int) code.length() && code[i] != '\n' )
		    result += code[i++];
	    } else if ( (code[i] == ':' && code[i + 1] == ':') ||
			(code[i] == '-' && code[i + 1] == '>') ) {
		result += '.';
		i += 2;
	    } else {
		result += code[i++];
	    }
	}
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
