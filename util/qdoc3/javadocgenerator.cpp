#include "javadocgenerator.h"

JavadocGenerator::JavadocGenerator()
    : oldDevice(0), currentDepth(0)
{
}

JavadocGenerator::~JavadocGenerator()
{
}

void JavadocGenerator::initializeGenerator(const Config &config)
{
    HtmlGenerator::initializeGenerator(config);
}

void JavadocGenerator::terminateGenerator()
{
    HtmlGenerator::terminateGenerator();
}

QString JavadocGenerator::format()
{
    return "javadoc";
}

void JavadocGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    HtmlGenerator::generateTree(tree, marker);
}

QString JavadocGenerator::fileExtension()
{
    return "jdoc";
}

void JavadocGenerator::startText(const Node *relative, CodeMarker *marker)
{
    oldDevice = out().device();
    out().setString(&buffer);
    HtmlGenerator::startText(relative, marker);
}

void JavadocGenerator::endText(const Node *relative, CodeMarker *marker)
{
    HtmlGenerator::endText(relative, marker);
    out().setDevice(oldDevice);

    buffer.replace("\"", "&quote;");
    out() << buffer;
    buffer.clear();
}

int JavadocGenerator::generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker)
{
    return HtmlGenerator::generateAtom(atom, relative, marker);
}

/*
    enum
    method
    variablesetter
    variablegetter
    enum-value
*/

void JavadocGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    generateIndent();
    out() << "<class name=\"" << protect(inner->name()) << "\"";
    generateDoc(inner, marker);
    out() << ">\n";

    ++currentDepth;
    foreach (Node *node, inner->childNodes()) {
        if (node->isInnerNode()) {
            generateClassLikeNode(static_cast<InnerNode *>(node), marker);
        } else {
            if (node->type() == Node::Enum) {
                generateIndent();
                out() << "<enum name=\"" << protect(node->name()) << "\"";
                generateDoc(node, marker);
                out() << "/>\n";
            } else if (node->type() == Node::Function) {
                generateIndent();
                out() << "<method name=\"" << protect(node->name()) << "\"";
                generateDoc(node, marker);
                out() << "/>\n";
            }
        }
    }
    --currentDepth;
    
    generateIndent();
    out() << "</class>\n";
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
    generateText(node->doc().body(), node, marker);
}

void JavadocGenerator::generateAlsoList(const Node *node, CodeMarker *marker)
{
    // ###
}

/*
    Neutralize dumb functions called from HtmlGenerator.
*/
void JavadocGenerator::generateDcf(const QString & /* fileBase */, const QString & /* startPage */,
                                   const QString & /* title */, DcfSection & /* dcfRoot */)
{
}

void JavadocGenerator::generateIndex(const QString & /* fileBase */, const QString & /* url */,
                                     const QString & /* title */)
{
}

void JavadocGenerator::generateIndent()
{
    for (int i = 0; i < currentDepth; ++i)
        out() << "    ";
}

void JavadocGenerator::generateDoc(const Node *node, CodeMarker *marker)
{
    if (!node->doc().body().isEmpty()) {
        out() << " doc=\"/**\n";
        generateText(node->doc().body(), node, marker);
        out() << "\n*/\"";
    }
}
