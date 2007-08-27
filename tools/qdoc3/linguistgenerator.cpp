/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  linguistgenerator.cpp
*/

#include "codemarker.h"
#include "pagegenerator.h"
#include "linguistgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"
#include <ctype.h>

#include <qlist.h>
#include <qiterator.h>

#define COMMAND_VERSION                 Doc::alias("version")

LinguistGenerator::LinguistGenerator()
    : PageGenerator()
{
}

LinguistGenerator::~LinguistGenerator()
{
}

void LinguistGenerator::initializeGenerator(const Config &config)
{
    Generator::initializeGenerator(config);
}

void LinguistGenerator::terminateGenerator()
{
    PageGenerator::terminateGenerator();
}

QString LinguistGenerator::format()
{
    return "Linguist";
}

QString LinguistGenerator::fileExtension(const Node * /* node */)
{
    return "ts";
}

void LinguistGenerator::generateClassLikeNode(const InnerNode *inner, CodeMarker *marker)
{
    out().setCodec("utf-8");

    QDomDocument document("TS");
    QDomElement documentElement = document.createElement("TS");
    documentElement.setAttribute("version", "1.1");

    QList<QDomElement> contextElements = generateIndexSections(document, inner, marker);
    foreach (QDomElement element, contextElements)
        documentElement.appendChild(element);

    QDomProcessingInstruction process = document.createProcessingInstruction(
        "xml", QString("version=\"1.0\" encoding=\"%1\"").arg("utf-8"));
    document.appendChild(process);
    document.appendChild(documentElement);

    out() << document;
    out().flush();
}

void LinguistGenerator::generateFakeNode( const FakeNode *fake, CodeMarker *marker )
{
    out().setCodec("utf-8");

    QDomDocument document("TS");
    QDomElement documentElement = document.createElement("TS");
    documentElement.setAttribute("version", "1.1");

    QList<QDomElement> contextElements = generateIndexSections(document, fake, marker);
    foreach (QDomElement element, contextElements)
        documentElement.appendChild(element);

    QDomProcessingInstruction process = document.createProcessingInstruction(
        "xml", QString("version=\"1.0\" encoding=\"%1\"").arg("utf-8"));
    document.appendChild(process);
    document.appendChild(documentElement);

    out() << document;
    out().flush();
}

QList<QDomElement> LinguistGenerator::generateIndexSections(
                   QDomDocument &document, const Node *node, CodeMarker *marker)
{
    QList<QDomElement> contexts;

    if (node->isInnerNode()) {
        const InnerNode *inner = static_cast<const InnerNode *>(node);

        foreach (Node *child, inner->childNodes()) {
            // Recurse to generate a DOM element for this child node and all
            // its children.
            contexts += generateIndexSections(document, child, marker);
        }
/*
        foreach (Node *child, inner->relatedNodes()) {
            QDomElement childElement = generateIndexSections(document, child, marker);
            element.appendChild(childElement);
        }
*/
    }

    // Add documentation to this node if it exists.
    if (!node->doc().isEmpty()) {

        QString nodeName = fullName(node);
        QString signature;

        if (node->type() == Node::Function) {
            QStringList pieces;
            const FunctionNode *functionNode = static_cast<const FunctionNode*>(node);
            foreach (Parameter parameter, functionNode->parameters()) {
                QString typeString = parameter.leftType() + parameter.rightType();
                if (typeString.split(" ").size() > 1)
                    pieces.append(typeString + parameter.name());
                else
                    pieces.append(typeString + " " + parameter.name());
            }
            signature = "(" + pieces.join(", ") + ")";
        }

        QDomElement contextElement = document.createElement("context");
        QDomElement nameElement = document.createElement("name");
        nameElement.appendChild(document.createTextNode(nodeName + signature));
        contextElement.appendChild(nameElement);

        QDomElement messageElement = document.createElement("message");
        contextElement.appendChild(messageElement);

        QDomElement sourceElement = document.createElement("source");
        QString sourceText = simplified(node->doc().source());
        if (!signature.isEmpty() && signature != "()" && !sourceText.contains("\\fn"))
            sourceText.prepend(QString("\\fn %1%2\n").arg(nodeName).arg(signature));
        sourceElement.appendChild(document.createTextNode(sourceText));
        messageElement.appendChild(sourceElement);

        QDomElement translationElement = document.createElement("translation");
        translationElement.setAttribute("type", "unfinished");
        messageElement.appendChild(translationElement);

        QDomElement locationElement = document.createElement("location");
        locationElement.setAttribute("filename", node->doc().location().filePath());
        locationElement.setAttribute("line", node->doc().location().lineNo());
        messageElement.appendChild(locationElement);

        contexts.append(contextElement);
    }

    return contexts;
}

QString LinguistGenerator::fullName(const Node *node) const
{
    if (!node)
        return "";
    else if (node->parent() && !node->parent()->name().isEmpty())
        return fullName(node->parent()) + "::" + node->name();
    else
        return node->name();
}

QString LinguistGenerator::simplified(const QString &text) const
{
    QStringList lines = text.split("\n");

    while (lines.size() > 0 && lines.first().trimmed().isEmpty())
        lines.pop_front();

    while (lines.size() > 0 && lines.last().trimmed().isEmpty())
        lines.pop_back();

    int min = 0;
    bool set = false;
    foreach (QString line, lines) {
        int j = 0;
        while (j < line.length()) {
            if (line[j] != ' ')
                break;
            ++j;
        }
        if (j < line.length()) {
            if (!set) {
                min = j;
                set = true;
            } else
                min = qMin(min, j);
        }
    }
    for (int i = 0; i < lines.size(); ++i)
        lines[i] = lines[i].mid(min);
    
    return lines.join("\n");
}
