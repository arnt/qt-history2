#include "javadocgenerator.h"

JavadocGenerator::JavadocGenerator()
{
}

JavadocGenerator::~JavadocGenerator()
{
}

void JavadocGenerator::initializeGenerator(const Config & /* config */)
{
}

void JavadocGenerator::terminateGenerator()
{
}

QString JavadocGenerator::format()
{
    return "javadoc";
}

void JavadocGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    HtmlGenerator::generateTree(tree, marker);
}

void JavadocGenerator::startText(const Node *relative, CodeMarker *marker)
{
    HtmlGenerator::startText(relative, marker);
}

void JavadocGenerator::endText(const Node *relative, CodeMarker *marker)
{
    HtmlGenerator::endText(relative, marker);
}

int JavadocGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    return HtmlGenerator::generateAtom(atom, relative, marker);
}

void JavadocGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    return HtmlGenerator::generateClassLikeNode(inner, marker);
}

void JavadocGenerator::generateFakeNode(const FakeNode *fake, CodeMarker *marker)
{
    return HtmlGenerator::generateFakeNode(fake, marker);
}

void JavadocGenerator::generateText(const Text& text, const Node *relative, CodeMarker *marker)
{
    return HtmlGenerator::generateText(text, relative, marker);
}

void JavadocGenerator::generateBody(const Node *node, CodeMarker *marker)
{
    return HtmlGenerator::generateBody(node, marker);
}

void JavadocGenerator::generateAlsoList(const Node *node, CodeMarker *marker)
{
    return HtmlGenerator::generateAlsoList(node, marker);
}
