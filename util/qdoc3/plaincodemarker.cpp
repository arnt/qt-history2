#include "plaincodemarker.h"

PlainCodeMarker::PlainCodeMarker()
{
}

PlainCodeMarker::~PlainCodeMarker()
{
}

bool PlainCodeMarker::recognizeCode( const QString& /* code */ )
{
    return true;
}

bool PlainCodeMarker::recognizeExtension( const QString& /* ext */ )
{
    return true;
}

bool PlainCodeMarker::recognizeLanguage( const QString& /* lang */ )
{
    return false;
}

QString PlainCodeMarker::plainName( const Node * /* node */ )
{
    return "";
}

QString PlainCodeMarker::plainFullName(const Node * /* node */, const Node * /* relative */)
{
    return "";
}

QString PlainCodeMarker::markedUpCode( const QString& code,
				       const Node * /* relative */,
				       const QString& /* dirPath */ )
{
    return protect( code );
}

QString PlainCodeMarker::markedUpSynopsis( const Node * /* node */,
					   const Node * /* relative */,
					   SynopsisStyle /* style */ )
{
    return "foo";
}

QString PlainCodeMarker::markedUpName( const Node * /* node */ )
{
    return "";
}

QString PlainCodeMarker::markedUpFullName( const Node * /* node */,
					   const Node * /* relative */ )
{
    return "";
}

QString PlainCodeMarker::markedUpEnumValue(const QString & /* enumValue */,
                                           const Node * /* relative */)
{
    return "";
}

QString PlainCodeMarker::markedUpIncludes( const QStringList& /* includes */ )
{
    return "";
}

QString PlainCodeMarker::functionBeginRegExp( const QString& /* funcName */ )
{
    return "";
}

QString PlainCodeMarker::functionEndRegExp( const QString& /* funcName */ )
{
    return "";
}

QList<Section> PlainCodeMarker::sections(const InnerNode * /* innerNode */,
                                         SynopsisStyle /* style */,
					 Status /* status */)
{
     return QList<Section>();
}

const Node *PlainCodeMarker::resolveTarget( const QString& /* target */,
                                            const Tree * /* tree */,
					    const Node * /* relative */ )
{
    return 0;
}
