/*
    jambiapiparser.h
*/

#ifndef JAMBIAPIPARSER_H
#define JAMBIAPIPARSER_H

#include <QStack>
#include <QXmlDefaultHandler>

#include "codeparser.h"

struct ClassOrEnumInfo
{
    QString tag;
    QString javaName;
    QString cppName;
    Node *javaNode;
    Node *cppNode;

    ClassOrEnumInfo() : javaNode(0), cppNode(0) {}
};

class JambiApiParser : public CodeParser, private QXmlDefaultHandler
{
public:
    JambiApiParser(Tree *cppTree);
    ~JambiApiParser();

    void initializeParser(const Config &config);
    void terminateParser();
    QString language();
    QString sourceFileNameFilter();
    void parseSourceFile(const Location &location, const QString &filePath, Tree *tree);
    virtual void doneParsingSourceFiles(Tree *tree);

private:
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool fatalError(const QXmlParseException &exception);
    void jambifyDocsPass2(Node *node);
    bool makeFunctionNode(InnerNode *parent, const QString &synopsis, FunctionNode **funcPtr);

    Tree *cppTre;
    Tree *javaTre;

    bool metJapiTag;
    Location japiLocation;
    QStack<ClassOrEnumInfo> classAndEnumStack;
};

#endif
