/*
  cpptoqsconverter.cpp
*/

#include "config.h"
#include "cpptoqsconverter.h"

#define CONFIG_QUICK                    "quick"
#define CONFIG_INDENTSIZE               "indentsize"

void setTabSize( int size );
void setIndentSize( int size );
int columnForIndex( const QString& t, int index );
int indentForBottomLine( const QStringList& program, QChar typedIn );

static QString balancedParens = "(?:[^()]+|\\([^()]*\\))*";

QRegExp CppToQsConverter::qClassRegExp;
QRegExp CppToQsConverter::addressOperatorRegExp;
QRegExp CppToQsConverter::gulbrandsenRegExp;
int CppToQsConverter::tabSize;

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

QString CppToQsConverter::convertedCode( Tree *qsTree, const QString& code,
					 const Set<QString>& classesWithNoQ )
{
    QString result;
    QStringList program;
    QStringList comments;
    int programWidth = 0;

    QStringList originalLines = QStringList::split( "\n", code, TRUE );
    QStringList::ConstIterator ol = originalLines.begin();
    while ( ol != originalLines.end() ) {
	QString code = (*ol).stripWhiteSpace();
	QString comment;

	int slashSlash = code.find( "//" );
	if ( slashSlash != -1 ) {
	    comment = code.mid( slashSlash );
	    code.truncate( slashSlash );
	    code = code.stripWhiteSpace();
	}

	code = convertCodeLine( qsTree, program, code, classesWithNoQ );
	program.append( code );

	comment = convertComment( qsTree, comment, classesWithNoQ );
	comments.append( comment );

	int n = indentForBottomLine( program, QChar::null );
	for ( int i = 0; i < n; i++ )
	    program.last().prepend( " " );

	int width = columnForIndex( program.last(), program.last().length() );
	if ( !comment.isEmpty() && width > programWidth )
	    programWidth = width;
	++ol;
    }

    programWidth = ( (programWidth + (tabSize - 1) + 2) / tabSize ) * tabSize;

    QStringList::ConstIterator p = program.begin();
    QStringList::ConstIterator c = comments.begin();
    while ( c != comments.end() ) {
	if ( c != comments.begin() )
	    result += "\n";

	if ( (*p).stripWhiteSpace().isEmpty() ) {
	    if ( !(*c).isEmpty() )
		result += *p;
	} else {
	    result += *p;
	    if ( !(*c).isEmpty() ) {
		int i = columnForIndex( *p, (*p).length() );
		while ( i++ < programWidth )
		    result += " ";
	    }
	}
	result += *c;
	++p;
	++c;
    }
    return result;
}

void CppToQsConverter::initialize( const Config& config )
{
    qClassRegExp.setPattern( "\\bQ([A-Z][A-Za-z]+)\\b" );
    addressOperatorRegExp.setPattern( "([(\\s])[*&]([a-zA-Z])" );
    gulbrandsenRegExp.setPattern( "\\b::\\b|->" );

    tabSize = config.getInt( CONFIG_TABSIZE );
    setTabSize( tabSize );

    int size = config.getInt( CONFIG_QUICK + Config::dot + CONFIG_INDENTSIZE );
    if ( size > 0 )
	setIndentSize( size );
}

void CppToQsConverter::terminate()
{
}

QString CppToQsConverter::convertCodeLine( Tree *qsTree,
					   const QStringList& /* program */,
					   const QString& code,
					   const Set<QString>& classesWithNoQ )
{
    static QString dataTypeFmt =
	"(?!return)(?:const\\b\\s*)?[A-Za-z_]+(?:\\s*[*&])?";
    static QRegExp funcPrototypeRegExp(
	"(" + dataTypeFmt + ")\\s*\\b([A-Z][a-zA-Z_0-9]*::)?"
	"([a-z][a-zA-Z_0-9]*)\\(([^);]*)(\\)?)(?:\\s*const)?" );
    static QRegExp paramRegExp(
	"^\\s*(" + dataTypeFmt + ")\\s*\\b([a-z][a-zA-Z_0-9]*)\\s*(,)?\\s*" );
    static QRegExp uninitVarRegExp(
	"(" + dataTypeFmt + ")\\s*\\b([a-z][a-zA-Z_0-9]*);" );
    static QRegExp eqVarRegExp(
	dataTypeFmt + "\\s*\\b([a-z][a-zA-Z_0-9]*)\\s*=(\\s*)(.*)" );
    static QRegExp ctorVarRegExp(
	"(" + dataTypeFmt + ")\\s*\\b([a-z][a-zA-Z_0-9]*)\\((.*)\\);" );

    if ( code.isEmpty() || code == "{" || code == "}" )
	return code;

    QString result;

    if ( funcPrototypeRegExp.exactMatch(code) ) {
	QString returnType = funcPrototypeRegExp.cap( 1 );
	QString className = funcPrototypeRegExp.cap( 2 );
	QString funcName = funcPrototypeRegExp.cap( 3 );
	QString params = funcPrototypeRegExp.cap( 4 ).stripWhiteSpace();
	bool toBeContinued = funcPrototypeRegExp.cap( 5 ).isEmpty();

	className.replace( "::", "." );

	result = "function " + className + funcName + "(";

	if ( !params.isEmpty() && params != "void" ) {
	    result += " ";
	    int i = funcPrototypeRegExp.pos( 4 );
	    while ( (i = paramRegExp.search(code, i,
					    QRegExp::CaretAtOffset)) != -1 ) {
		QString dataType = paramRegExp.cap( 1 );
		QString paramName = paramRegExp.cap( 2 );
		QString comma = paramRegExp.cap( 3 );

		result += paramName + " : " +
			  convertedDataType( qsTree, dataType );
		if ( comma.isEmpty() )
		    break;
		result += ", ";
		i += paramRegExp.matchedLength();
	    }
	    result += " ";
	}

	result += ")";
	returnType = convertedDataType( qsTree, returnType );
	if ( !returnType.isEmpty() )
	    result += " : " + returnType;
    } else if ( uninitVarRegExp.exactMatch(code) ) {
	QString dataType = uninitVarRegExp.cap( 1 );
	QString varName = uninitVarRegExp.cap( 2 );

	result = "var " + varName;
	dataType = convertedDataType( qsTree, dataType );
	if ( !dataType.isEmpty() )
	    result += " : " + dataType;
	result += ";";
    } else if ( eqVarRegExp.exactMatch(code) ) {
	QString varName = eqVarRegExp.cap( 1 );
	QString value = eqVarRegExp.cap( 3 );

	value = convertExpr( qsTree, value, classesWithNoQ );
	result += "var " + varName + " = " + value;
    } else if ( ctorVarRegExp.exactMatch(code) ) {
	QString dataType = ctorVarRegExp.cap( 1 );
	QString varName = ctorVarRegExp.cap( 2 );
	QString value = ctorVarRegExp.cap( 3 ).stripWhiteSpace();

	result += "var " + varName + " = ";

	dataType = convertedDataType( qsTree, dataType );
	value = convertExpr( qsTree, value, classesWithNoQ );

	if ( dataType.isEmpty() || dataType == "String" ) {
	    if ( value.contains(",") ) {
		result += "...";
	    } else {
		result += value;
	    }
	} else {
	    result += "new " + dataType;
	    if ( !value.isEmpty() )
		result += "( " + value + " )";
	}
	result += ";";
    } else {
	result = convertExpr( qsTree, code, classesWithNoQ );
    }
    return result;
}

QString CppToQsConverter::convertComment( Tree * /* qsTree */,
					  const QString& comment,
					  const Set<QString>& classesWithNoQ )

{
    QString result = comment;

    result.replace( "TRUE", "true" );
    result.replace( "FALSE", "false" );
    result.replace( addressOperatorRegExp, "\\1\\2" );
    result.replace( gulbrandsenRegExp, "." );

    int i = 0;
    while ( (i = result.find(qClassRegExp, i)) != -1 ) {
	if ( classesWithNoQ.contains(qClassRegExp.cap(1)) )
	    result.remove( i, 1 );
	i++;
    }
    return result;
}

QString CppToQsConverter::convertExpr( Tree *qsTree, const QString& expr,
				       const Set<QString>& classesWithNoQ )
{
    // not entirely right
    return convertComment( qsTree, expr, classesWithNoQ );
}
