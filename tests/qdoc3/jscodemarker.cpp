/*
  jscodemarker.cpp
*/

#include "jscodemarker.h"
#include "node.h"

JsCodeMarker::JsCodeMarker()
{
}

JsCodeMarker::~JsCodeMarker()
{
}

QString JsCodeMarker::markedUpCode( const QString& code,
				    const Node * /* relative */,
				    const QString& /* dirPath */ ) const
{
    return protect( code );
}

QString JsCodeMarker::markedUpSynopsys( const Node * /* node */,
					const Node * /* relative */,
					SynopsysStyle /* style */ ) const
{
    return "";
}

QString JsCodeMarker::markedUpName( const Node *node ) const
{
    QString name = linkTag( node, taggedNode(node) );
    if ( node->type() == Node::Function )
	name += "()";
    return name;
}

QString JsCodeMarker::markedUpFullName( const Node *node ) const
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

QString JsCodeMarker::markedUpIncludes(
	const QStringList& /* includes */ ) const
{
    return "";
}

const Node *JsCodeMarker::resolveTarget( const QString& /* target */,
					 const Node * /* relative */ ) const
{
    return 0;
}

bool JsCodeMarker::recognizeCode( const QString& /* code */ ) const
{
    return FALSE;
}

bool JsCodeMarker::recognizeExtension( const QString& ext ) const
{
    return ext == "js" || ext == "qs";
}

bool JsCodeMarker::recognizeLanguage( const QString& lang ) const
{
    return lang == "javascript";
}
