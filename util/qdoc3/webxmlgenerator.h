/*
  WebXMLGenerator.h
*/

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>
#include <qdom.h>

#include "codemarker.h"
#include "config.h"
#include "pagegenerator.h"

class WebXMLGenerator : public PageGenerator
{
public:
    WebXMLGenerator();
    ~WebXMLGenerator();

    virtual void initializeGenerator( const Config& config );
    virtual void terminateGenerator();
    virtual QString format();
    virtual void generateTree(const Tree *tree, CodeMarker *marker);

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual int generateAtom( const Atom *atom, const Node *relative,
			      CodeMarker *marker );
    virtual void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );
    virtual QString fileExtension(const Node *node);

    virtual const Atom *addAtomElements(QDomElement &parent, const Atom *atom,
                                 const Node *relative, CodeMarker *marker);
    QDomElement generateIndexSections(QDomDocument &document, const Node *node,
                                      CodeMarker *marker);

private:
    const Tree *tre;
    bool inLink;
    bool inContents;
    bool inSectionHeading;
    bool inTableHeader;
    int numTableRows;
    bool threeColumnEnumValueTable;
    QString link;
    QStringList sectionNumber;
};

#endif
