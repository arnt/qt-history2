/*
  webxmlgenerator.cpp
*/

#include "codemarker.h"
#include "htmlgenerator.h"
#include "webxmlgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"
#include <ctype.h>

#include <qlist.h>
#include <qiterator.h>
#include <qdebug.h>

#define COMMAND_VERSION                 Doc::alias("version")

static bool showBrokenLinks = false;

WebXMLGenerator::WebXMLGenerator()
    : HtmlGenerator()
{
}

WebXMLGenerator::~WebXMLGenerator()
{
}

void WebXMLGenerator::initializeGenerator(const Config &config)
{
    Generator::initializeGenerator(config);
}

void WebXMLGenerator::terminateGenerator()
{
    HtmlGenerator::terminateGenerator();
}

QString WebXMLGenerator::format()
{
    return "WebXML";
}

void WebXMLGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    tre = tree;
    HtmlGenerator::generateTree(tree, marker);
}

void WebXMLGenerator::startText(const Node *relative, CodeMarker *marker)
{
    HtmlGenerator::startText(relative, marker);
}

int WebXMLGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    int skipAhead = 0;
    static bool in_para = false;

    switch (atom->type()) {
    default:
        HtmlGenerator::generateAtom(atom, relative, marker);
    }
    return skipAhead;
}

void WebXMLGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    tre->generateIndexSection("", out(), inner, true);
}

void WebXMLGenerator::generateFakeNode( const FakeNode *fake, CodeMarker *marker )
{
    tre->generateIndexSection("", out(), fake, true);
}

QString WebXMLGenerator::fileExtension()
{
    return "xml";
}
