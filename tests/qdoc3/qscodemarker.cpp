/*
  qscodemarker.cpp
*/

#include "node.h"
#include "qscodemarker.h"

QsCodeMarker::QsCodeMarker()
{
}

QsCodeMarker::~QsCodeMarker()
{
}

bool QsCodeMarker::recognizeCode( const QString& /* code */ )
{
    return FALSE;
}

bool QsCodeMarker::recognizeExtension( const QString& ext )
{
    return ext == "js" || ext == "qs";
}

bool QsCodeMarker::recognizeLanguage( const QString& lang )
{
    return lang == "JavaScript" || lang == "Qt Script";
}

QString QsCodeMarker::markedUpCode( const QString& code,
				    const Node * /* relative */,
				    const QString& /* dirPath */ )
{
    return protect( code );
}

QString QsCodeMarker::markedUpSynopsis( const Node *node,
					const Node * /* relative */,
					SynopsisStyle style )
{
    const FunctionNode *func;
    const PropertyNode *property;
    QString synopsis;
    QStringList extras;
    QString name;

    name = taggedNode( node );
    if ( style == Summary )
        name = linkTag( node, name );
    name = "<@name>" + name + "</@name>";
    if ( style == Detailed && !node->parent()->name().isEmpty() )
	name.prepend( taggedNode(node->parent()) + "." );

    switch ( node->type() ) {
    case Node::Class:
        synopsis = "class " + name;
        break;
    case Node::Function:
        func = (const FunctionNode *) node;
	if ( style == Summary )
	    synopsis = "function ";
	synopsis += name + " (";
        if ( !func->parameters().isEmpty() ) {
            synopsis += " ";
            QValueList<Parameter>::ConstIterator p = func->parameters().begin();
            while ( p != func->parameters().end() ) {
                if ( p != func->parameters().begin() )
                    synopsis += ", ";
		synopsis += "var";
		if ( !(*p).name().isEmpty() )
		    synopsis += " <@param>" + protect( (*p).name() ) +
				"</@param>";
		synopsis += " : " + protect( (*p).leftType() );
                ++p;
            }
            synopsis += " ";
        }
        synopsis += ")";
	if ( !func->returnType().isEmpty() )
	    synopsis += " : " + protect( func->returnType() );

        if ( style == Detailed ) {
            if ( func->metaness() == FunctionNode::Signal )
                extras << "[signal]";
        }
        break;
    case Node::Property:
        property = (const PropertyNode *) node;
	if ( style == Summary )
	    synopsis = "var ";
	synopsis += name + " : " + property->dataType();
	if ( property->setter().isEmpty() )
	    extras << "(read only)";
        break;
    case Node::Namespace:
    case Node::Enum:
    case Node::Typedef:
    default:
        synopsis = name;
    }

    if ( style == Summary ) {
        if ( node->status() == Node::Preliminary ) {
            extras << "(preliminary)";
        } else if ( node->status() == Node::Deprecated ) {
            extras << "(deprecated)";
        } else if ( node->status() == Node::Obsolete ) {
            extras << "(obsolete)";
        }
    }

    QString extra;
    if ( !extras.isEmpty() )
        extra = "<@extra>" + extras.join(" ") + "</@extra>";
    return synopsis + extra;
}

QString QsCodeMarker::markedUpName( const Node *node )
{
    QString name = linkTag( node, taggedNode(node) );
    if ( node->type() == Node::Function )
	name += "()";
    return name;
}

QString QsCodeMarker::markedUpFullName( const Node *node,
					const Node * /* relative */ )
{
    QString fullName;
    for ( ;; ) {
	fullName.prepend( markedUpName(node) );
	if ( node->parent()->name().isEmpty() )
	    break;
	node = node->parent();
	fullName.prepend( "<@op>.</@op>" );
    }
    return fullName;
}

QString QsCodeMarker::markedUpIncludes( const QStringList& /* includes */ )
{
    return "";
}

QString QsCodeMarker::functionBeginRegExp( const QString& funcName )
{
    return "^function[ \t].*\\b" +
#if QT_VERSION >= 0x030100
	   QRegExp::escape
#endif
	   ( funcName );
}

QString QsCodeMarker::functionEndRegExp( const QString& /* funcName */ )
{
    return "^}";
}

QValueList<ClassSection> QsCodeMarker::classSections( const ClassNode *classe,
						      SynopsisStyle style )
{
    QValueList<ClassSection> sections;
    QString suffix;
    if ( style == Detailed )
	suffix += "' Documentation";
    FastClassSection enums( "Enums" + suffix );
    FastClassSection functions( "Functions" + suffix );
    FastClassSection properties( "Properties" + suffix );
    FastClassSection signalz( "Signals" + suffix );

    NodeList::ConstIterator c = classe->childNodes().begin();
    while ( c != classe->childNodes().end() ) {
	if ( (*c)->type() == Node::Enum ) {
	    insert( enums, *c );
	} else if ( (*c)->type() == Node::Function ) {
	    const FunctionNode *func = (const FunctionNode *) *c;
	    if ( func->metaness() == FunctionNode::Signal ) {
		insert( signalz, *c );
	    } else {
		insert( functions, *c );
	    }
	} else if ( (*c)->type() == Node::Property ) {
	    insert( properties, *c );
	}
	++c;
    }
    append( sections, enums );
    append( sections, properties );
    append( sections, functions );
    append( sections, signalz );
    return sections;
}

const Node *QsCodeMarker::resolveTarget( const QString& /* target */,
					 const Node * /* relative */ )
{
    return 0;
}
