/*
  codemarker.h
*/

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include <qregexp.h>

class Config;
class Node;

class CodeMarker
{
public:
    enum SynopsisStyle { Overview, Detailed };

    CodeMarker();
    virtual ~CodeMarker();

    virtual void initializeMarker( const Config& config );
    virtual void terminateMarker();

    virtual bool recognizeCode( const QString& code ) = 0;
    virtual bool recognizeExtension( const QString& ext ) = 0;
    virtual bool recognizeLanguage( const QString& lang ) = 0;
    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath ) = 0;
    virtual QString markedUpSynopsis( const Node *node, const Node *relative,
				      SynopsisStyle style ) = 0;
    virtual QString markedUpName( const Node *node ) = 0;
    virtual QString markedUpFullName( const Node *node,
				      const Node *relative ) = 0;
    virtual QString markedUpIncludes( const QStringList& includes ) = 0;
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative ) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static CodeMarker *markerForCode( const QString& code );
    static CodeMarker *markerForFileName( const QString& fileName );
    static CodeMarker *markerForLanguage( const QString& lang );
    static const Node *nodeForString( const QString& string );
    static QString stringForNode( const Node *node );

protected:
    QString protect( const QString& string );
    QString taggedNode( const Node *node );
    QString linkTag( const Node *node, const QString& body );

private:
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;

    static QString defaultLang;
    static QValueList<CodeMarker *> markers;
};

#endif
