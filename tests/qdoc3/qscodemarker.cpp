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
    QString extra;
    QString name;

    name = taggedNode( node );
    if ( style == Overview )
        name = linkTag( node, name );
    name = "<@name>" + name + "</@name>";

    switch ( node->type() ) {
    case Node::Class:
        synopsis = "class " + name;
        break;
    case Node::Function:
        func = (const FunctionNode *) node;
        synopsis = "function " + name + " (";
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
		if ( style == Detailed )
		    synopsis += " : " + protect( (*p).leftType() );
                ++p;
            }
            synopsis += " ";
        }
        synopsis += ")";
	if ( !func->returnType().isEmpty() )
	    synopsis += " : " + protect( func->returnType() );

        if ( style == Detailed ) {
            QStringList bracketed;
            if ( func->metaness() == FunctionNode::Signal ) {
                bracketed += "signal";
            } else if ( func->metaness() == FunctionNode::Slot ) {
                bracketed += "slot";
            }
            if ( !bracketed.isEmpty() )
                extra += " [" + bracketed.join(" ") + "]";
        }
        break;
    case Node::Property:
        property = (const PropertyNode *) node;
        synopsis = property->dataType() + " " + name;
        if ( style == Overview ) {
            if ( property->setter().isEmpty() )
                extra += " (read only)";
        }
        break;
    case Node::Namespace:
    case Node::Enum:
    case Node::Typedef:
    default:
        synopsis = name;
    }

    if ( style == Overview ) {
        if ( node->status() == Node::Preliminary ) {
            extra += " (preliminary)";
        } else if ( node->status() == Node::Deprecated ) {
            extra += " (deprecated)";
        } else if ( node->status() == Node::Obsolete ) {
            extra += " (obsolete)";
        }
    }

    if ( !extra.isEmpty() ) {
        extra.prepend( "<@extra>" );
        extra.append( "</@extra>" );
    }
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
	fullName += markedUpName( node );
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

const Node *QsCodeMarker::resolveTarget( const QString& /* target */,
					 const Node * /* relative */ )
{
    return 0;
}
