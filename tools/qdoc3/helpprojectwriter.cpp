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
#include <QHash>

#include "helpprojectwriter.h"
#include "config.h"
#include "node.h"
#include "tree.h"

QT_BEGIN_NAMESPACE

HelpProjectWriter::HelpProjectWriter(const Config &config, const QString &defaultFileName)
{
    // The output directory should already have been checked by the calling
    // generator.
    outputDir = config.getString(CONFIG_OUTPUTDIR);

    helpNamespace = config.getString(CONFIG_QHP + Config::dot + "namespace");
    virtualFolder = config.getString(CONFIG_QHP + Config::dot + "virtualFolder");
    fileName = config.getString(CONFIG_QHP + Config::dot + "file");
    if (fileName.isEmpty())
        fileName = defaultFileName;
    extraFiles = config.getStringList(CONFIG_QHP + Config::dot + "extraFiles").toSet();
    indexPage = config.getString(CONFIG_QHP + Config::dot + "indexPage");
    indexTitle = config.getString(CONFIG_QHP + Config::dot + "indexTitle");
    filterAttributes = config.getStringList(CONFIG_QHP + Config::dot + "filterAttributes").toSet();
    QSet<QString> customFilterNames = config.subVars(CONFIG_QHP + Config::dot + "customFilters");
    foreach (QString filterName, customFilterNames) {
        QString name = config.getString(CONFIG_QHP + Config::dot + "customFilters" + Config::dot + filterName + Config::dot + "name");
        QSet<QString> filters = config.getStringList(CONFIG_QHP + Config::dot + "customFilters" + Config::dot + filterName + Config::dot + "filterAttributes").toSet();
        customFilters[name] = filters;
    }
    //customFilters = config.defs.
}

void HelpProjectWriter::addExtraFile(const QString &file)
{
    extraFiles.insert(file);
}

void HelpProjectWriter::addExtraFiles(const QSet<QString> &files)
{
    extraFiles.unite(files);
}

QStringList HelpProjectWriter::keywordDetails(const Node *node) const
{
    QStringList details;
    details << node->name();

    if (node->parent() && !node->parent()->name().isEmpty())
        details << node->parent()->name()+"::"+node->name();
    else
        details << node->name();
    details << tree->fullDocumentName(node);

    return details;
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

    QString href = tree->fullDocumentName(node);

    switch (node->type()) {

        case Node::Class:
            if (href != indexPage) {
                writer.writeStartElement("section");
                writer.writeAttribute("ref", href);
                writer.writeAttribute("title", objName);
                writer.writeEndElement(); // section

                keywords.append(keywordDetails(node));
                files.insert(tree->fullDocumentName(node));
            }
            break;

        case Node::Namespace:
            if (href != indexPage) {
                writer.writeStartElement("section");
                writer.writeAttribute("ref", href);
                writer.writeAttribute("title", objName);
                writer.writeEndElement(); // section

                keywords.append(keywordDetails(node));
                files.insert(tree->fullDocumentName(node));
            }
            break;

        case Node::Function:
            {
                keywords.append(keywordDetails(node));
            }
            break;

        // Fake nodes (such as manual pages) contain subtypes, titles and other
        // attributes.
        case Node::Fake: {
            const FakeNode *fakeNode = static_cast<const FakeNode*>(node);
            if (href != indexPage) {
                if (fakeNode->subType() != FakeNode::ExternalPage) {
                    if (!fakeNode->links().contains(Node::ContentsLink)) {
                        writer.writeStartElement("section");
                        writer.writeAttribute("ref", href);
                        writer.writeAttribute("title", fakeNode->title());
                        writer.writeEndElement(); // section

                        if (fakeNode->doc().hasKeywords()) {
                            foreach (Atom *keyword, fakeNode->doc().keywords()) {
                                QStringList details;
                                details << keyword->string()
                                        << keyword->string()
                                        << tree->fullDocumentName(node) + "#" + Doc::canonicalTitle(keyword->string());
                                keywords.append(details);
                            }
                        }
                        if (fakeNode->doc().hasTableOfContents()) {
                            foreach (Atom *item, fakeNode->doc().tableOfContents()) {
                                QString title = Text::sectionHeading(item).toString();
                                QStringList details;
                                details << title
                                        << title
                                        << tree->fullDocumentName(node) + "#" + Doc::canonicalTitle(title);
                                keywords.append(details);
                            }
                        }

                        keywords.append(keywordDetails(node));
                        files.insert(tree->fullDocumentName(node));
                    }
                }
            }
            break;
            }
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

void HelpProjectWriter::generate(const Tree *tre)
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
    QHash<QString, QSet<QString> >::ConstIterator it;
    for (it = customFilters.begin(); it != customFilters.end(); ++it) {
        writer.writeStartElement("customFilter");
        writer.writeAttribute("name", it.key());
        foreach (QString filter, it.value())
            writer.writeTextElement("filterAttribute", filter);
        writer.writeEndElement(); // customFilter
    }

    // Start the filterSection.
    writer.writeStartElement("filterSection");

    // Write filterAttribute elements.
    foreach (QString filterName, filterAttributes)
        writer.writeTextElement("filterAttribute", filterName);

    writer.writeStartElement("toc");
    writer.writeStartElement("section");
    writer.writeAttribute("ref", indexPage);
    writer.writeAttribute("title", indexTitle);
    files.insert(indexPage);

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

QT_END_NAMESPACE
