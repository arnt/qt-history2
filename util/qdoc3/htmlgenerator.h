/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>

#include "codemarker.h"
#include "config.h"
#include "dcfsection.h"
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

    static QString protect( const QString& string );
    static QString protectPreformatted( const QString& string );

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileExtension();

private:
    const QPair<QString,QString> anchorForNode(const Node *node);
    const Node *findNodeForTarget(const QString &target, const Node *relative,
                                  CodeMarker *marker, const Atom *atom = 0);
    void generateHeader(const QString& title, const Node *node = 0,
                        CodeMarker *marker = 0);
    void generateTitle( const QString& title, const QString &subTitle = QString());
    void generateFooter( const Node *node = 0 );
    void generateBrief(const Node *node, CodeMarker *marker);
    void generateIncludes(const InnerNode *inner, CodeMarker *marker);
#if 0
    void generateNavigationBar( const NavigationBar& bar, const Node *node,
    				CodeMarker *marker );
#endif
    void generateTableOfContents(const Node *node, CodeMarker *marker,
                                 Doc::SectioningUnit sectioningUnit,
                                 int numColumns, const Node *relative = 0);
    QString generateListOfAllMemberFile(const InnerNode *inner, CodeMarker *marker);
    QString generateLowStatusMemberFile(const InnerNode *inner, CodeMarker *marker,
                                        CodeMarker::Status status);
    void generateClassHierarchy(const Node *relative, CodeMarker *marker,
				const QMap<QString, const Node *> &classMap);
    void generateAnnotatedList(const Node *relative, CodeMarker *marker,
			       const QMap<QString, const Node *> &nodeMap);
    void generateCompactList(const Node *relative, CodeMarker *marker,
			     const QMap<QString, const Node *> &classMap);
    void generateFunctionIndex(const Node *relative, CodeMarker *marker);
    void generateLegaleseList(const Node *relative, CodeMarker *marker);
    void generateOverviewList(const Node *relative, CodeMarker *marker);
    void generateSynopsis(const Node *node, const Node *relative, CodeMarker *marker,
			  CodeMarker::SynopsisStyle style);
    void generateSectionList(const Section& section, const Node *relative,
			     CodeMarker *marker, CodeMarker::SynopsisStyle style);
    void generateSectionInheritedList(const Section& section, const Node *relative,
                                      CodeMarker *marker);
    void generateFullName(const Node *apparentNode, const Node *relative, CodeMarker *marker,
			  const Node *actualNode = 0);
    void generateDetailedMember(const Node *node, const InnerNode *relative, CodeMarker *marker);
    void generateLink(const Atom *atom, const Node *relative, CodeMarker *marker);

    QString cleanRef( const QString& ref );
    QString registerRef( const QString& ref );
    QString highlightedCode( const QString& markedCode, CodeMarker *marker, const Node *relative );
    QString fileBase(const Node *node);
#if 0
    QString fileBase( const Node *node, const SectionIterator& section );
#endif
    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );
    QString refForAtom(Atom *atom, const Node *node);
    void findAllClasses(const InnerNode *node);
    void findAllFunctions(const InnerNode *node);
    void findAllLegaleseTexts(const InnerNode *node);
    static int hOffset(const Node *node);
    static bool isThreeColumnEnumValueTable(const Atom *atom);
    QString getLink(const Atom *atom, const Node *relative, CodeMarker *marker);
    void generateDcf(const QString &fileBase, const QString &startPage, const QString &title,
                     DcfSection &dcfRoot);

#if 0
    NavigationBar currentNavigationBar;
#endif
    QMap<QString, QString> refMap;
    DcfSection dcfClassesRoot;
    DcfSection dcfOverviewsRoot;
    DcfSection dcfExamplesRoot;
    DcfSection dcfDesignerRoot;
    DcfSection dcfLinguistRoot;
    DcfSection dcfAssistantRoot;
    DcfSection dcfQmakeRoot;
    bool inLink;
    bool inContents;
    bool inSectionHeading;
    bool inTableHeader;
    int numTableRows;
    bool threeColumnEnumValueTable;
    QString link;
    QStringList sectionNumber;
    QRegExp funcLeftParen;
    QString style;
    QString postHeader;
    QString footer;
    QString address;
    QString project;
    QString navigationLinks;
    const Tree *tre;
    QMap<QString, QMap<QString, const Node *> > moduleClassMap;
    QMap<QString, const Node *> nonCompatClasses;
    QMap<QString, const Node *> mainClasses;
    QMap<QString, const Node *> compatClasses;
    QMap<QString, QMap<QString, const Node *> > funcIndex;
    QMap<Text, const Node *> legaleseTexts;
};

#define HTMLGENERATOR_ADDRESS           "address"
#define HTMLGENERATOR_FOOTER            "footer"
#define HTMLGENERATOR_POSTHEADER        "postheader"
#define HTMLGENERATOR_STYLE             "style"

#endif
