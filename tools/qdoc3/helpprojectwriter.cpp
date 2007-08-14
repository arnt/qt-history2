/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtXml>

#include "helpprojectwriter.h"
#include "node.h"
#include "tree.h"

HelpProjectWriter::HelpProjectWriter(const QHash<QString, QString> defs)
{
    helpNamespace = defs.value("namespace");
    virtualFolder = defs.value("virtualFolder");
    fileName = defs.value("file");
    extraFiles = defs.value("extraFiles").split(" ", QString::SkipEmptyParts).toSet();
    indexPage = defs.value("indexPage");
    indexTitle = defs.value("indexTitle");
}

void HelpProjectWriter::addExtraFile(const QString &file)
{
    extraFiles.insert(file);
}

void HelpProjectWriter::addExtraFiles(const QSet<QString> &files)
{
    extraFiles.unite(files);
}

bool HelpProjectWriter::generateSection(QXmlStreamWriter &writer, const Node *node)
{
    if (!node->url().isEmpty())
        return false;

    if (node->access() == Node::Private)
        return false;

    QString objName = node->name();
    if (objName.isEmpty())
        return true;

    switch (node->type()) {

        case Node::Class:
            writer.writeStartElement("section");
            writer.writeAttribute("ref", tree->fullDocumentName(node));
            writer.writeAttribute("title", objName);
            writer.writeEndElement(); // section
            files.insert(tree->fullDocumentName(node));
            break;

        case Node::Namespace:
            writer.writeStartElement("section");
            writer.writeAttribute("ref", tree->fullDocumentName(node));
            writer.writeAttribute("title", objName);
            writer.writeEndElement(); // section
            files.insert(tree->fullDocumentName(node));
            break;

        case Node::Function:
            {
                QStringList details;
                details << objName;
                if (node->parent())
                    details << node->parent()->name()+"::"+objName;
                else
                    details << objName;
                details << tree->fullDocumentName(node);
                keywords.append(details);
            }
            break;

        // Fake nodes (such as manual pages) contain subtypes, titles and other
        // attributes.
        case Node::Fake:
            const FakeNode *fakeNode = static_cast<const FakeNode*>(node);
            if (fakeNode->subType() != FakeNode::ExternalPage) {
                if (!fakeNode->links().contains(Node::ContentsLink)) {
                    writer.writeStartElement("section");
                    writer.writeAttribute("ref", tree->fullDocumentName(node));
                    writer.writeAttribute("title", fakeNode->title());
                    writer.writeEndElement(); // section
                    files.insert(tree->fullDocumentName(node));
                }
            }
            break;

        default:
            break;
    }
    return true;
}

void HelpProjectWriter::generateSections(QXmlStreamWriter &writer, const Node *node)
{
    if (!generateSection(writer, node))
        return;

    if (node->isInnerNode()) {
        const InnerNode *inner = static_cast<const InnerNode *>(node);

        foreach (Node *child, inner->childNodes())
            generateSections(writer, child);
    }
}

void HelpProjectWriter::generate(const Tree *tre, const QString &outputDir)
{
    this->tree = tre;

    QFile file(outputDir + QDir::separator() + fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("QtHelpProject");
    writer.writeAttribute("version", "1.0");

    // Write metaData, virtualFolder and namespace elements.
    writer.writeTextElement("namespace", helpNamespace);
    writer.writeTextElement("virtualFolder", virtualFolder);
    // Write customFilter elements.

    // Start the filterSection.
    writer.writeStartElement("filterSection");

    // Write filterAttribute elements.

    writer.writeStartElement("toc");
    writer.writeStartElement("section");
    writer.writeAttribute("ref", indexPage);
    writer.writeAttribute("title", indexTitle);

    generateSections(writer, tree->root());

    writer.writeEndElement(); // section
    writer.writeEndElement(); // toc

    writer.writeStartElement("keywords");
    foreach (QStringList details, keywords) {
        writer.writeStartElement("keyword");
        writer.writeAttribute("name", details[0]);
        writer.writeAttribute("id", details[1]);
        writer.writeAttribute("ref", details[2]);
        writer.writeEndElement(); //keyword
    }
    writer.writeEndElement(); // keywords

    writer.writeStartElement("files");
    foreach (QString usedFile, files)
        writer.writeTextElement("file", usedFile);
    foreach (QString usedFile, extraFiles)
        writer.writeTextElement("file", usedFile);
    writer.writeEndElement(); // files

    writer.writeEndElement(); // filterSection
    writer.writeEndElement(); // QtHelpProject
    writer.writeEndDocument();
    file.close();
}
