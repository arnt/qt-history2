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
    QString synopsis;
    QStringList extras;
    QString name;

    name = taggedNode( node );
    if ( style == Summary )
        name = linkTag( node, name );
    name = "<@name>" + name + "</@name>";

    if ( style == Detailed && !node->parent()->name().isEmpty() &&
	 node->type() != Node::Enum )
	name.prepend( taggedNode(node->parent()) + "." );

    switch ( node->type() ) {
    case Node::Class:
        synopsis = "class " + name;
        break;
    case Node::Function:
	{
            const FunctionNode *func = (const FunctionNode *) node;

	    if ( style == Summary )
		synopsis = "function ";
	    synopsis += name + " (";

            if ( !func->parameters().isEmpty() ) {
        	synopsis += " ";
        	QValueList<Parameter>::ConstIterator p =
			func->parameters().begin();
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
	}
        break;
    case Node::Property:
	{
            const PropertyNode *property = (const PropertyNode *) node;

	    if ( style == Summary )
		synopsis = "var ";
	    synopsis += name + " : " + property->dataType();
	    if ( style == Detailed ) {
		if ( property->setter().isEmpty() )
		    extras << "[read only]";
	    }
	}
        break;
    case Node::Enum:
	{
	    // X or x occur in 0X32 or 0x32
	    QRegExp letterRegExp( "[A-WYZa-wyz]" );
	    const EnumNode *enume = (const EnumNode *) node;

	    synopsis = name;
	    if ( style == Summary && !enume->items().isEmpty() ) {
		synopsis += " : ";
		QString comma;
		QValueList<EnumItem>::ConstIterator it = enume->items().begin();
		while ( it != enume->items().end() ) {
		    if ( enume->itemAccess((*it).name()) == Node::Public ) {
			synopsis += comma;
			synopsis += (*it).name();
			if ( (*it).value().find(letterRegExp) != -1 )
			    synopsis += " = " + (*it).value();
			comma = ", ";
		    }
		    ++it;
		}
	    }
	}
	break;
    case Node::Namespace:
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
    if ( style == Summary ) {
	FastClassSection enums( "Enums" );
	FastClassSection functions( "Functions" );
	FastClassSection properties( "Properties" );
	FastClassSection readOnlyProperties( "Read-only Properties" );
	FastClassSection signalz( "Signals" );
	FastClassSection writableProperties( "Writable Properties" );

	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->access() == Node::Public ) {
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
		    const PropertyNode *property = (const PropertyNode *) *c;
		    if ( property->setter().isEmpty() ) {
			insert( readOnlyProperties, *c );
		    } else {
			insert( writableProperties, *c );
		    }
		}
	    }
	    ++c;
	}
	append( sections, enums );
	append( sections, writableProperties );
	append( sections, readOnlyProperties );
	append( sections, functions );
	append( sections, signalz );
    } else {
	FastClassSection enums( "Enum Documentation" );
	FastClassSection functionsAndSignals(
		"Function and Signal Documentation" );
	FastClassSection properties( "Property Documentation" );

	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->type() == Node::Enum ) {
		insert( enums, *c );
	    } else if ( (*c)->type() == Node::Function ) {
		insert( functionsAndSignals, *c );
	    } else if ( (*c)->type() == Node::Property ) {
		insert( properties, *c );
	    }
	    ++c;
	}
	append( sections, enums );
	append( sections, properties );
	append( sections, functionsAndSignals );
    }
    return sections;
}

const Node *QsCodeMarker::resolveTarget( const QString& /* target */,
					 const Node * /* relative */ )
{
    return 0;
}
