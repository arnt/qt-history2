/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>

#include "pagegenerator.h"
#include "sectioniterator.h"

struct NavigationBar
{
    SectionIterator prev;
    SectionIterator current;
    SectionIterator next;
};

class HtmlGenerator : public PageGenerator
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
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					const CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    const CodeMarker *marker );
    virtual void generateFakeNode( const FakeNode *fake,
				   const CodeMarker *marker );
    virtual QString fileBase( const Node *node );
    virtual QString fileExtension( const Node *node );

private:
    void generateHeader( const QString& title, const Node *node = 0 );
    void generateTitle( const QString& title );
    void generateFooter( const Node *node = 0 );
    void generateNavigationBar( const NavigationBar& bar, const Node *node,
    				const CodeMarker *marker );
    void generateListOfAllMemberFunctions( const ClassNode *classe,
					   const CodeMarker *marker );
    void generateSynopsis( const Node *node, const InnerNode *relative,
			   const CodeMarker *marker,
			   CodeMarker::SynopsisStyle style );
    QString cleanRef( const QString& ref );
    QString registerRef( const QString& ref );
    QString protect( const QString& string );
    QString highlightedCode( const QString& markedCode, const Node *relative );
    QString fileBase( const Node *node, const SectionIterator& section );
    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );

    NavigationBar currentNavigationBar;
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
