/*
    javacodemarker.cpp
*/

#include "javacodemarker.h"
#include "node.h"

JavaCodeMarker::JavaCodeMarker()
{
}

JavaCodeMarker::~JavaCodeMarker()
{
}

bool JavaCodeMarker::recognizeCode( const QString& /* code */ )
{
    return true;
}

bool JavaCodeMarker::recognizeExtension( const QString& ext )
{
    return ext == "java";
}

bool JavaCodeMarker::recognizeLanguage( const QString& lang )
{
    return lang == "Java";
}

QString JavaCodeMarker::plainName( const Node *node )
{
    return node->name();
}

QString JavaCodeMarker::plainFullName( const Node *node, const Node * /* relative */ )
{
    QString fullName;
    for ( ;; ) {
	fullName.prepend( plainName(node) );
	if ( node->parent()->name().isEmpty() )
	    break;
	node = node->parent();
	fullName.prepend(".");
    }
    return fullName;
}

QString JavaCodeMarker::markedUpCode( const QString& code,
				      const Node * /* relative */,
				      const QString& /* dirPath */ )
{
    return protect( code );
}

QString JavaCodeMarker::markedUpSynopsis(const Node * /* node */,
					 const Node * /* relative */,
					 SynopsisStyle /* style */)
{
    return QString();
}

QString JavaCodeMarker::markedUpName( const Node *node )
{
    return linkTag(node, taggedNode(node));
}

QString JavaCodeMarker::markedUpFullName(const Node *node, const Node * /* relative */ )
{
    QString fullName;
    for ( ;; ) {
	fullName.prepend( markedUpName(node) );
	if ( node->parent()->name().isEmpty() )
	    break;
	node = node->parent();
	fullName.prepend( "." );
    }
    return fullName;
}

QString JavaCodeMarker::markedUpEnumValue(const QString & /* enumValue */,
                                          const Node * /* relative */)
{
    return QString();
}

QString JavaCodeMarker::markedUpIncludes( const QStringList& /* includes */ )
{
    return QString();
}

QString JavaCodeMarker::functionBeginRegExp( const QString& funcName )
{
    return "^x$"; // ### invalid regexp
}

QString JavaCodeMarker::functionEndRegExp( const QString& /* funcName */ )
{
    return "^}";
}

QList<Section> JavaCodeMarker::sections(const InnerNode * /* inner */, SynopsisStyle /* style */,
                                        Status /* status */)
{
    return QList<Section>();
}

const Node *JavaCodeMarker::resolveTarget( const QString& /* target */,
                                         const Tree * /* tree */,
					 const Node * /* relative */ )
{
    return 0;
}
