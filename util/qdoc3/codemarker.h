/*
  codemarker.h
*/

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include <qpair.h>
#include <QRegExp>

#include "node.h"

class Config;
class Tree;

struct Section
{
    QString name;
    QString singularMember;
    QString pluralMember;
    NodeList members;
    QList<QPair<ClassNode *, int> > inherited;

    Section() { }
    Section( const QString& name0, const QString& singularMember0, const QString& pluralMember0 )
	: name( name0 ), singularMember( singularMember0 ),
	  pluralMember( pluralMember0 ) { }
};

struct FastSection
{
    const InnerNode *innerNode;
    QString name;
    QString singularMember;
    QString pluralMember;
    QMap<QString, Node *> memberMap;
    QList<QPair<ClassNode *, int> > inherited;

    FastSection( const InnerNode *innerNode0, const QString& name0 = "",
		      const QString& singularMember0 = "member",
		      const QString& pluralMember0 = "members" )
	: innerNode( innerNode0 ), name( name0 ), singularMember( singularMember0 ),
	  pluralMember( pluralMember0 ) { }
};

class CodeMarker
{
public:
    enum SynopsisStyle { Summary, Detailed, SeparateList, Accessors };
    enum Status { Compat, Obsolete, Okay };

    CodeMarker();
    virtual ~CodeMarker();

    virtual void initializeMarker( const Config& config );
    virtual void terminateMarker();
    virtual bool recognizeCode( const QString& code ) = 0;
    virtual bool recognizeExtension( const QString& ext ) = 0;
    virtual bool recognizeLanguage( const QString& lang ) = 0;
    virtual QString plainName( const Node *node ) = 0;
    virtual QString plainFullName( const Node *node, const Node *relative = 0 ) = 0;
    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath ) = 0;
    virtual QString markedUpSynopsis( const Node *node, const Node *relative,
				      SynopsisStyle style ) = 0;
    virtual QString markedUpName( const Node *node ) = 0;
    virtual QString markedUpFullName( const Node *node,
				      const Node *relative = 0 ) = 0;
    virtual QString markedUpEnumValue(const QString &enumValue, const Node *relative) = 0;
    virtual QString markedUpIncludes( const QStringList& includes ) = 0;
    virtual QString functionBeginRegExp( const QString& funcName ) = 0;
    virtual QString functionEndRegExp( const QString& funcName ) = 0;
    virtual QList<Section> sections(const InnerNode *inner, SynopsisStyle style, Status status) = 0;
    virtual const Node *resolveTarget(const QString& target, const Tree *tree,
		                      const Node *relative) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static CodeMarker *markerForCode( const QString& code );
    static CodeMarker *markerForFileName( const QString& fileName );
    static CodeMarker *markerForLanguage( const QString& lang );
    static const Node *nodeForString( const QString& string );
    static QString stringForNode( const Node *node );

protected:
    virtual QString sortName( const Node *node );
    QString protect(const QString &string);
    QString typified(const QString &string);
    QString taggedNode( const Node *node );
    QString linkTag( const Node *node, const QString& body );
    void insert(FastSection &fastSection, Node *node, SynopsisStyle style, Status status);
    void append( QList<Section>& sectionList, const FastSection& fastSection );

private:
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;

    static QString defaultLang;
    static QList<CodeMarker *> markers;
};

#endif
