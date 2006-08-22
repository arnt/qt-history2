/*
  WebXMLGenerator.h
*/

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include <qmap.h>
#include <qregexp.h>

#include "codemarker.h"
#include "config.h"
#include "htmlgenerator.h"

class WebXMLGenerator : public HtmlGenerator
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
    virtual QString fileExtension();

private:
    const Tree *tre;
};

#endif
