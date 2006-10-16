#ifndef JAVADOCGENERATOR_H
#define JAVADOCGENERATOR_H

#include "htmlgenerator.h"

class JavadocGenerator : public HtmlGenerator
{
public:
    JavadocGenerator();
    ~JavadocGenerator();

    void initializeGenerator(const Config &config);
    void terminateGenerator();
    QString format();
    void generateTree(const Tree *tree, CodeMarker *marker);

protected:
    QString fileExtension(const Node *node);
    void startText( const Node *relative, CodeMarker *marker );
    void endText( const Node *relative, CodeMarker *marker );
    int generateAtom( const Atom *atom, const Node *relative, CodeMarker *marker );
    void generateClassLikeNode(const InnerNode *inner, CodeMarker *marker);
    void generateFakeNode( const FakeNode *fake, CodeMarker *marker );

    void generateText( const Text& text, const Node *relative, CodeMarker *marker );
    void generateBody( const Node *node, CodeMarker *marker );
    void generateAlsoList( const Node *node, CodeMarker *marker );

    QString refForNode( const Node *node );
    QString linkForNode( const Node *node, const Node *relative );
    QString refForAtom(Atom *atom, const Node *node);

private:
    void generateDcf(const QString &fileBase, const QString &startPage,
                     const QString &title, DcfSection &dcfRoot);
    void generateIndex(const QString &fileBase, const QString &url,
                       const QString &title);
    void generateIndent();
    void generateDoc(const Node *node, CodeMarker *marker);
    void generateDoc(const Text &text, const Node *node, CodeMarker *marker);

    QString buffer;
    QIODevice *oldDevice;
    int currentDepth;
};

#endif
