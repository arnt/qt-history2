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
    if ( style != Detailed )
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

	    synopsis = name;

	    if ( style == SeparateList ) {
		synopsis += "()";
	    } else {
		synopsis += " (";
        	if ( !func->parameters().isEmpty() ) {
        	    synopsis += " ";
		    int numOptional = 0;
        	    QList<Parameter>::ConstIterator p = func->parameters().begin();
        	    while ( p != func->parameters().end() ) {
			if ( !(*p).defaultValue().isEmpty() ) {
			    if ( p == func->parameters().begin() ) {
				synopsis += "[ ";
			    } else {
				synopsis += " [ , ";
			    }
			    numOptional++;
			} else {
                	    if ( p != func->parameters().begin() )
                		synopsis += ", ";
			}
			if ( !(*p).name().isEmpty() )
			    synopsis += "<@param>" + protect( (*p).name() ) +
					"</@param> : ";
			synopsis += protect( (*p).leftType() );
                	++p;
        	    }
		    for ( int i = 0; i < numOptional; i++ )
			synopsis += " ]";
        	    synopsis += " ";
        	}
		synopsis += ")";
	    }

	    if ( style != SeparateList && !func->returnType().isEmpty() )
		synopsis += " : " + protect( func->returnType() );

            if ( style == Detailed && func->metaness() == FunctionNode::Signal )
		extras << "[signal]";
	}
        break;
    case Node::Property:
	{
            const PropertyNode *property = (const PropertyNode *) node;

	    synopsis = name;
	    if ( style != SeparateList )
		synopsis += " : " + property->dataType();
	    if ( style == Detailed && !property->setter() )
		extras << "[read only]";
	}
        break;
    case Node::Enum:
	{
	    /*
	      The letters A to F and X (upper- and lower-case) can
	      appear in a hexadecimal constant (e.g. 0x3F).
	    */
	    QRegExp letterRegExp( "[G-WYZg-wyz_]" );
	    const EnumNode *enume = (const EnumNode *) node;

	    synopsis = name;
	    if ( style == Summary && !enume->items().isEmpty() ) {
		synopsis += " : ";
		QString comma;
		QList<EnumItem>::ConstIterator it = enume->items().begin();
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
    return "^function[ \t].*\\b" + QRegExp::escape( funcName );
}

QString QsCodeMarker::functionEndRegExp( const QString& /* funcName */ )
{
    return "^}";
}

QList<ClassSection> QsCodeMarker::classSections( const ClassNode *classe, SynopsisStyle style )
{
    QList<ClassSection> sections;
    if ( style == Summary ) {
	FastClassSection enums(classe, "Enums", "enum", "enums");
	FastClassSection functions(classe, "Functions", "function", "functions");
	FastClassSection readOnlyProperties(classe, "Read-Only Properties", "property",
					    "properties");
	FastClassSection signalz(classe, "Signals", "signal", "signals");
	FastClassSection writableProperties(classe, "Writable Properties", "property",
					    "properties");

	QStack<const ClassNode *> stack;
	stack.push( classe );

	while ( !stack.isEmpty() ) {
	    const ClassNode *ancestorClass = stack.pop();

	    NodeList::ConstIterator c = ancestorClass->childNodes().begin();
	    while ( c != ancestorClass->childNodes().end() ) {
		if ( (*c)->access() == Node::Public ) {
		    if ( (*c)->type() == Node::Enum ) {
			insert( enums, *c, style );
		    } else if ( (*c)->type() == Node::Function ) {
			const FunctionNode *func = (const FunctionNode *) *c;
			if ( func->metaness() == FunctionNode::Signal ) {
			    insert( signalz, *c, style );
			} else {
			    insert( functions, *c, style );
			}
		    } else if ( (*c)->type() == Node::Property ) {
			const PropertyNode *property =
				(const PropertyNode *) *c;
			if ( property->setter() ) {
			    insert( writableProperties, *c, style );
			} else {
			    insert( readOnlyProperties, *c, style );
			}
		    }
		}
		++c;
	    }

	    QList<RelatedClass>::ConstIterator r = ancestorClass->baseClasses().begin();
	    while ( r != ancestorClass->baseClasses().end() ) {
		stack.prepend( (*r).node );
		++r;
	    }
	}
	append( sections, enums );
	append( sections, writableProperties );
	append( sections, readOnlyProperties );
	append( sections, functions );
	append( sections, signalz );
    } else if ( style == Detailed ) {
	FastClassSection enums( classe, "Enum Documentation" );
	FastClassSection functionsAndSignals( classe,
		"Function and Signal Documentation" );
	FastClassSection properties( classe, "Property Documentation" );

	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->access() == Node::Public ) {
		if ( (*c)->type() == Node::Enum ) {
		    insert( enums, *c, style );
		} else if ( (*c)->type() == Node::Function ) {
		    insert( functionsAndSignals, *c, style );
		} else if ( (*c)->type() == Node::Property ) {
		    insert( properties, *c, style );
		}
	    }
	    ++c;
	}
	append( sections, enums );
	append( sections, properties );
	append( sections, functionsAndSignals );
    } else { // ( style == SeparateList )
	FastClassSection all( classe );

	QStack<const ClassNode *> stack;
	stack.push( classe );

	while ( !stack.isEmpty() ) {
	    const ClassNode *ancestorClass = stack.pop();

	    NodeList::ConstIterator c = ancestorClass->childNodes().begin();
	    while ( c != ancestorClass->childNodes().end() ) {
		if ( (*c)->access() == Node::Public )
		    insert( all, *c, style );
		++c;
	    }

	    QList<RelatedClass>::ConstIterator r = ancestorClass->baseClasses().begin();
	    while ( r != ancestorClass->baseClasses().end() ) {
		stack.prepend( (*r).node );
		++r;
	    }
	}
	append( sections, all );
    }
    return sections;
}

const Node *QsCodeMarker::resolveTarget( const QString& /* target */,
					 const Node * /* relative */ )
{
    return 0;
}
