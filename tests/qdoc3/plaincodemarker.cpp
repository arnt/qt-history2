/*
  plaincodemarker.cpp
*/

#include "plaincodemarker.h"

PlainCodeMarker::PlainCodeMarker()
{
}

PlainCodeMarker::~PlainCodeMarker()
{
}

bool PlainCodeMarker::recognizeCode( const QString& /* code */ )
{
    return TRUE;
}

bool PlainCodeMarker::recognizeExtension( const QString& /* ext */ )
{
    return TRUE;
}

bool PlainCodeMarker::recognizeLanguage( const QString& /* lang */ )
{
    return FALSE;
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

QList<ClassSection> PlainCodeMarker::classSections(
	const ClassNode * /* classe */, SynopsisStyle /* style */ )
{
    return QList<ClassSection>();
}

const Node *PlainCodeMarker::resolveTarget( const QString& /* target */,
					    const Node * /* relative */ )
{
    return 0;
}
