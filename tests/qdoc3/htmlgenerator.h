/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>

#include "webgenerator.h"

class QTextStream;

class HtmlGenerator : public WebGenerator
{
public:
    HtmlGenerator();
    ~HtmlGenerator();

    virtual QString formatString() const;

protected:
    virtual void startMolecule( const Node *relative,
				const CodeMarker *marker );
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       const CodeMarker *marker );
    virtual QString fileBase( const Node *node );
    virtual QString fileExtension( const Node *node );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					const CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    const CodeMarker *marker );

private:
    void generateHeader( QTextStream& outStream );
    void generateFooter( QTextStream& outStream );
    void generateListOfAllMemberFunctions( const ClassNode *classe,
					   const CodeMarker *marker );
    void generateSynopsis( const Node *node, const InnerNode *relative,
			   const CodeMarker *marker,
			   CodeMarker::SynopsisStyle style );
    QString cleanRef( const QString& ref );
    QString registerRef( const QString& ref );
    QString protect( const QString& string );
    QString highlightedCode( const QString& markedCode, const Node *relative );
    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );

    QMap<QString, QString> refMap;
    bool inLink;
    QString link;
    QRegExp funcLeftParen;
    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
};

#endif
