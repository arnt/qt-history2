/*
  codemarker.h
*/

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include <qregexp.h>

#include "node.h"

class Config;

struct ClassSection
{
    QString name;
    NodeList members;

    ClassSection() { }
    ClassSection( const QString& name0 )
	: name( name0 ) { }
};

struct FastClassSection
{
    QString name;
    QMap<QString, Node *> memberMap;

    FastClassSection( const QString& name0 )
	: name( name0 ) { }
};

class CodeMarker
{
public:
    enum SynopsisStyle { Summary, Detailed };

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
    virtual QString functionBeginRegExp( const QString& funcName ) = 0;
    virtual QString functionEndRegExp( const QString& funcName ) = 0;
    virtual QValueList<ClassSection> classSections( const ClassNode *classe,
						    SynopsisStyle style ) = 0;
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
    virtual QString sortName( const Node *node );
    QString protect( const QString& string );
    QString taggedNode( const Node *node );
    QString linkTag( const Node *node, const QString& body );
    void insert( FastClassSection& fastSection, Node *node );
    void append( QValueList<ClassSection>& sectionList,
		 const FastClassSection& fastSection );

private:
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;

    static QString defaultLang;
    static QValueList<CodeMarker *> markers;
};

#endif
