/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>

#include "config.h"
#include "pagegenerator.h"

#if 0
struct NavigationBar
{
    SectionIterator prev;
    SectionIterator current;
    SectionIterator next;
};
#endif

class HtmlGenerator : public PageGenerator
{
public:
    HtmlGenerator();
    ~HtmlGenerator();

    virtual void initializeGenerator( const Config& config );
    virtual void terminateGenerator();
    virtual QString format();
    virtual void generateTree(const Tree *tree, CodeMarker *marker);

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    CodeMarker *marker );
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileBase( const Node *node );
    virtual QString fileExtension( const Node *node );

private:
    void generateHeader( const QString& title, const Node *node = 0 );
    void generateTitle( const QString& title );
    void generateFooter( const Node *node = 0 );
#if 0
    void generateNavigationBar( const NavigationBar& bar, const Node *node,
    				CodeMarker *marker );
#endif
    QString generateListOfAllMemberFile( const ClassNode *classe,
					 CodeMarker *marker );
    void generateClassHierarchy(const Node *relative, CodeMarker *marker,
				const QMap<QString, const Node *> &classMap);
    void generateAnnotatedList(const Node *relative, CodeMarker *marker,
			       const QMap<QString, const Node *> &classMap);
    void generateCompactList(const Node *relative, CodeMarker *marker,
			     const QMap<QString, const Node *> &classMap);
    void generateFunctionIndex(const Node *relative, CodeMarker *marker);
    void generateLegaleseList(const Node *relative, CodeMarker *marker);
    void generateSynopsis(const Node *node, const InnerNode *relative, CodeMarker *marker,
			  CodeMarker::SynopsisStyle style);
    void generateClassSectionList(const ClassSection& section, const ClassNode *relative,
				  CodeMarker *marker, CodeMarker::SynopsisStyle style);
    void generateFullName(const Node *apparentNode, const Node *relative, CodeMarker *marker,
			  const Node *actualNode = 0);

    QString cleanRef( const QString& ref );
    QString registerRef( const QString& ref );
    QString protect( const QString& string );
    QString highlightedCode( const QString& markedCode, const Node *relative );
#if 0
    QString fileBase( const Node *node, const SectionIterator& section );
#endif
    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );
    void findAllClasses(const InnerNode *node);
    void findAllFunctions(const InnerNode *node);
    void findAllLegaleseTexts(const InnerNode *node);
    static int hOffset(const Node *node);

#if 0
    NavigationBar currentNavigationBar;
#endif
    QMap<QString, QString> refMap;
    bool inLink;
    QString link;
    QRegExp funcLeftParen;
    QString style;
    QString postHeader;
    QString footer;
    QString address;
    QString version;
    const InnerNode *root;
    QMap<QString, const Node *> allClasses;
    QMap<QString, const Node *> mainClasses;
    QMap<QString, QMap<QString, const Node *> > funcIndex;
    QMap<Text, const Node *> legaleseTexts;
};

#define HTMLGENERATOR_ADDRESS           "address"
#define HTMLGENERATOR_FOOTER            "footer"
#define HTMLGENERATOR_POSTHEADER        "postheader"
#define HTMLGENERATOR_STYLE             "style"

#endif
