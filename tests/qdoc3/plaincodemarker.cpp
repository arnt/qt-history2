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

QString PlainCodeMarker::markedUpCode( const QString& code,
				       const Node * /* relative */,
				       const QString& /* dirPath */ ) const
{
    return protect( code );
}

QString PlainCodeMarker::markedUpSynopsis( const Node * /* node */,
					   const Node * /* relative */,
					   SynopsisStyle /* style */ ) const
{
    return "";
}

QString PlainCodeMarker::markedUpName( const Node * /* node */ ) const
{
    return "";
}

QString PlainCodeMarker::markedUpFullName( const Node * /* node */,
					   const Node * /* relative */ ) const
{
    return "";
}

QString PlainCodeMarker::markedUpIncludes(
	const QStringList& /* includes */ ) const
{
    return "";
}

const Node *PlainCodeMarker::resolveTarget( const QString& /* target */,
					    const Node * /* relative */ ) const
{
    return 0;
}

bool PlainCodeMarker::recognizeCode( const QString& /* code */ ) const
{
    return TRUE;
}

bool PlainCodeMarker::recognizeExtension( const QString& /* ext */ ) const
{
    return TRUE;
}

bool PlainCodeMarker::recognizeLanguage( const QString& /* lang */ ) const
{
    return TRUE;
}
