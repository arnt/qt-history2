/*
  codemarker.h
*/

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include <qregexp.h>

class Node;

class CodeMarker
{
public:
    enum SynopsysStyle { Overview, Detailed };

    CodeMarker();
    virtual ~CodeMarker();

    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath ) const = 0;
    virtual QString markedUpSynopsys( const Node *node, const Node *relative,
				      SynopsysStyle style ) const = 0;
    virtual QString markedUpName( const Node *node ) const = 0;
    virtual QString markedUpFullName( const Node *node ) const = 0;
    virtual QString markedUpIncludes( const QStringList& includes ) const = 0;
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative ) const = 0;
    virtual bool recognizeCode( const QString& code ) const = 0;
    virtual bool recognizeExtension( const QString& ext ) const = 0;
    virtual bool recognizeLanguage( const QString& lang ) const = 0;

    static const CodeMarker *markerForCode( const QString& code,
					    const QString& defaultLang );
    static const CodeMarker *markerForFileName( const QString& fileName,
						const QString& defaultLang );
    static const CodeMarker *markerForLanguage( const QString& lang );
    static const Node *nodeForString( const QString& string );
    static QString stringForNode( const Node *node );

protected:
    QString protect( const QString& string ) const;
    QString taggedNode( const Node *node ) const;
    QString linkTag( const Node *node, const QString& body ) const;

private:
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;

    static QValueList<const CodeMarker *> markers;
};

#endif
