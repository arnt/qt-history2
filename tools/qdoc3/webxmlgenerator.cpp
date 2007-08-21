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

/*
  webxmlgenerator.cpp
*/

#include <QtXml>

#include "codemarker.h"
#include "pagegenerator.h"
#include "webxmlgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"

#define COMMAND_VERSION                 Doc::alias("version")

WebXMLGenerator::WebXMLGenerator()
    : PageGenerator()
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
    PageGenerator::terminateGenerator();
}

QString WebXMLGenerator::format()
{
    return "WebXML";
}

QString WebXMLGenerator::fileExtension(const Node * /* node */)
{
    return "xml";
}

void WebXMLGenerator::generateTree(const Tree *tree, CodeMarker *marker)
{
    tre = tree;
    PageGenerator::generateTree(tree, marker);
}

void WebXMLGenerator::startText(const Node *relative, CodeMarker *marker)
{
    inLink = false;
    inContents = false;
    inSectionHeading = false;
    numTableRows = 0;
    sectionNumber.clear();
    PageGenerator::startText(relative, marker);
}

int WebXMLGenerator::generateAtom(QXmlStreamWriter &writer, const Atom *atom,
                                  const Node *relative, CodeMarker *marker)
{
    int skipAhead = 0;

    switch (atom->type()) {
    default:
        PageGenerator::generateAtom(atom, relative, marker);
    }
    return skipAhead;
}

void WebXMLGenerator::generateClassLikeNode(const InnerNode *inner,
                                            CodeMarker *marker)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("WebXML");
    writer.writeStartElement("document");

    generateIndexSections(writer, inner, marker);

    writer.writeEndElement(); // document
    writer.writeEndElement(); // WebXML
    writer.writeEndDocument();

    out() << data;
    out().flush();
}

void WebXMLGenerator::generateFakeNode(const FakeNode *fake, CodeMarker *marker)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("WebXML");
    writer.writeStartElement("document");

    generateIndexSections(writer, fake, marker);

    writer.writeEndElement(); // document
    writer.writeEndElement(); // WebXML
    writer.writeEndDocument();

    out() << data;
    out().flush();
}

void WebXMLGenerator::generateIndexSections(QXmlStreamWriter &writer,
                                 const Node *node, CodeMarker *marker)
{
    if (tre->generateIndexSection(writer, node)) {

        // Add documentation to this node if it exists.
        writer.writeStartElement("description");
        startText(node, marker);

        const Atom *atom = node->doc().body().firstAtom();
        while (atom)
            atom = addAtomElements(writer, atom, node, marker);
        writer.writeEndElement(); // description

        if (node->isInnerNode()) {
            const InnerNode *inner = static_cast<const InnerNode *>(node);

            // Recurse to generate an element for this child node and all its children.
            foreach (Node *child, inner->childNodes())
                generateIndexSections(writer, child, marker);
/*
            foreach (Node *child, inner->relatedNodes()) {
                QDomElement childElement = generateIndexSections(document, child, marker);
                element.appendChild(childElement);
            }
*/
        }
        writer.writeEndElement();
    }
}

const Atom *WebXMLGenerator::addAtomElements(QXmlStreamWriter &writer,
     const Atom *atom, const Node *relative, CodeMarker *marker)
{
    switch (atom->type()) {
    case Atom::AbstractLeft:
    case Atom::AbstractRight:
        break;
    case Atom::AutoLink:
        if (true) {
            writer.writeStartElement("link");
            writer.writeAttribute("href", atom->string());
            writer.writeCharacters(atom->string());
            writer.writeEndElement(); // link
        }
        break;
    case Atom::BaseName:
        break;
    case Atom::BriefLeft:

        writer.writeStartElement("brief");
        if (relative->type() == Node::Property)
            writer.writeCharacters("This property holds ");
        else
            writer.writeCharacters("This variable holds ");

        atom = atom->next();
        while (atom && atom->type() != Atom::BriefRight)
            atom = addAtomElements(writer, atom, relative, marker);

        if (relative->type() == Node::Property || relative->type() == Node::Variable)
            writer.writeCharacters(".");

        writer.writeEndElement(); // brief
        break;

    case Atom::BriefRight:
        break;

    case Atom::C:
        writer.writeStartElement("teletype");
        if (inLink)
            writer.writeAttribute("type", "normal");
        else
            writer.writeAttribute("type", "highlighted");

        writer.writeCharacters(plainCode(atom->string()));
        writer.writeEndElement(); // teletype
        break;

    case Atom::Code:
        writer.writeTextElement("code", trimmedTrailing(plainCode(atom->string())));
        break;

    case Atom::CodeBad:
        writer.writeTextElement("badcode", trimmedTrailing(plainCode(atom->string())));
        break;

    case Atom::CodeNew:
        writer.writeTextElement("para", "you can rewrite it as");
        writer.writeTextElement("newcode", trimmedTrailing(plainCode(atom->string())));
        break;

    case Atom::CodeOld:
        writer.writeTextElement("para", "For example, if you have code like");
        writer.writeTextElement("oldcode", trimmedTrailing(plainCode(atom->string())));
        break;

    case Atom::FootnoteLeft:

        writer.writeStartElement("footnote");

        atom = atom->next();
        while (atom && atom->type() != Atom::FootnoteRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // footnote
        break;

    case Atom::FootnoteRight:
        break;

    case Atom::FormatElse:
    case Atom::FormatEndif:
    case Atom::FormatIf:
        break;
    case Atom::FormattingLeft:
/*        out() << formattingLeftMap()[atom->string()];
        if ( atom->string() == ATOM_FORMATTING_PARAMETER ) {
            if ( atom->next() != 0 && atom->next()->type() == Atom::String ) {
                QRegExp subscriptRegExp( "([a-z]+)_([0-9n])" );
                if ( subscriptRegExp.exactMatch(atom->next()->string()) ) {
                    out() << subscriptRegExp.cap( 1 ) << "<sub>"
                          << subscriptRegExp.cap( 2 ) << "</sub>";
                    skipAhead = 1;
                }
            }
        }*/
        break;
    case Atom::FormattingRight:
/*        if ( atom->string() == ATOM_FORMATTING_LINK ) {
            if (inLink) {
                if ( link.isEmpty() ) {
                    if (showBrokenLinks)
                        out() << "</i>";
                } else {
                    out() << "</a>";
                }
            }
            inLink = false;
        } else {
            out() << formattingRightMap()[atom->string()];
        }*/
        break;
    case Atom::GeneratedList:
/*        if (atom->string() == "annotatedclasses") {
            generateAnnotatedList(relative, marker, nonCompatClasses);
        } else if (atom->string() == "classes") {
            generateCompactList(relative, marker, nonCompatClasses);
        } else if (atom->string().contains("classesbymodule")) {
            QString arg = atom->string().trimmed();
            QString moduleName = atom->string().mid(atom->string().indexOf(
                "classesbymodule") + 15).trimmed();
            if (moduleClassMap.contains(moduleName))
                generateAnnotatedList(relative, marker, moduleClassMap[moduleName]);
        } else if (atom->string().contains("classesbyedition")) {
            QString arg = atom->string().trimmed();
            QString editionName = atom->string().mid(atom->string().indexOf(
                "classesbyedition") + 16).trimmed();
            if (editionModuleMap.contains(editionName)) {
                QMap<QString, const Node *> editionClasses;
                foreach (QString moduleName, editionModuleMap[editionName]) {
                    if (moduleClassMap.contains(moduleName))
                        editionClasses.unite(moduleClassMap[moduleName]);
                }
                generateAnnotatedList(relative, marker, editionClasses);
            }
        } else if (atom->string() == "classhierarchy") {
            generateClassHierarchy(relative, marker, nonCompatClasses);
        } else if (atom->string() == "compatclasses") {
            generateCompactList(relative, marker, compatClasses);
        } else if (atom->string() == "functionindex") {
            generateFunctionIndex(relative, marker);
        } else if (atom->string() == "legalese") {
            generateLegaleseList(relative, marker);
        } else if (atom->string() == "mainclasses") {
            generateCompactList(relative, marker, mainClasses);
        } else if (atom->string() == "services") {
            generateCompactList(relative, marker, serviceClasses);
        } else if (atom->string() == "overviews") {
            generateOverviewList(relative, marker);
        } else if (atom->string() == "namespaces") {
            generateAnnotatedList(relative, marker, namespaceIndex);
        } else if (atom->string() == "related") {
            const FakeNode *fake = static_cast<const FakeNode *>(relative);
            if (fake && !fake->groupMembers().isEmpty()) {
                QMap<QString, const Node *> groupMembersMap;
                foreach (Node *node, fake->groupMembers()) {
                    if (node->type() == Node::Fake)
                        groupMembersMap[node->name()] = node;
                }
                generateAnnotatedList(fake, marker, groupMembersMap);
            }
        }
*/
        break;
    case Atom::Image:
        writer.writeStartElement("image");
        writer.writeAttribute("href", imageFileName(relative, atom->string()));
        writer.writeEndElement(); // image
        break;

    case Atom::InlineImage:
        writer.writeStartElement("inlineimage");
        writer.writeAttribute("href", imageFileName(relative, atom->string()));
        writer.writeEndElement(); // inlineimage
        break;

    case Atom::ImageText:
        break;

    case Atom::LegaleseLeft:
        writer.writeStartElement("legalese");

        atom = atom->next();
        while (atom && atom->type() != Atom::LegaleseRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // legalese
        break;

    case Atom::LegaleseRight:
        break;

    case Atom::Link:
        writer.writeStartElement("link");
        writer.writeAttribute("href", atom->string());
        //inLink = true;
        writer.writeEndElement(); // link
        break;

    case Atom::LinkNode:
        writer.writeStartElement("link");
        writer.writeAttribute("href", atom->string());
        //inLink = true;
        writer.writeEndElement(); // link
        break;

    case Atom::ListLeft:
        writer.writeStartElement("list");

        if (atom->string() == ATOM_LIST_BULLET)
            writer.writeAttribute("type", "bullet");
        else if (atom->string() == ATOM_LIST_TAG)
            writer.writeAttribute("type", "definition");
        else if (atom->string() == ATOM_LIST_VALUE)
            writer.writeAttribute("type", "enum");
        else {
            writer.writeAttribute("type", "ordered");
            if (atom->string() == ATOM_LIST_UPPERALPHA)
                writer.writeAttribute("start", "A");
            else if (atom->string() == ATOM_LIST_LOWERALPHA)
                writer.writeAttribute("start", "a");
            else if (atom->string() == ATOM_LIST_UPPERROMAN)
                writer.writeAttribute("start", "I");
            else if (atom->string() == ATOM_LIST_LOWERROMAN)
                writer.writeAttribute("start", "i");
            else // (atom->string() == ATOM_LIST_NUMERIC)
                writer.writeAttribute("start", "1");
        }

        atom = atom->next();
        while (atom && atom->type() != Atom::ListRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // list
        break;

    case Atom::ListItemNumber:
        break;

    case Atom::ListTagLeft:
        {
            writer.writeStartElement("definition");

            writer.writeTextElement("term", plainCode(
                marker->markedUpEnumValue(atom->next()->string(), relative)));

            atom = atom->next();
            while (atom && atom->type() != Atom::ListTagRight)
                atom = addAtomElements(writer, atom, relative, marker);
        }

        writer.writeEndElement(); // definition
        break;

    case Atom::ListTagRight:
        break;

    case Atom::ListItemLeft:
        writer.writeStartElement("item");

        atom = atom->next();
        while (atom && atom->type() != Atom::ListItemRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // item
        break;

    case Atom::ListItemRight:
        break;

    case Atom::ListRight:
        break;

    case Atom::Nop:
        break;

    case Atom::ParaLeft:
        writer.writeStartElement("para");

        atom = atom->next();
        while (atom && atom->type() != Atom::ParaRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // para
        break;

    case Atom::ParaRight:
        break;

    case Atom::QuotationLeft:
        writer.writeStartElement("quote");

        atom = atom->next();
        while (atom && atom->type() != Atom::QuotationRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // quote
        break;

    case Atom::QuotationRight:
        break;

    case Atom::RawString:
        writer.writeCharacters(atom->string());
        break;

    case Atom::SectionLeft:
        writer.writeStartElement("section");

        writer.writeCharacters(Doc::canonicalTitle(Text::sectionHeading(atom).toString()));
        atom = atom->next();
        while (atom && atom->type() != Atom::SectionRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // section
        break;

    case Atom::SectionRight:
        break;

    case Atom::SectionHeadingLeft:
        writer.writeStartElement("heading");
        writer.writeAttribute("level", atom->string()); // + hOffset(relative)

        inSectionHeading = true;
        atom = atom->next();
        while (atom && atom->type() != Atom::SectionHeadingRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // heading
        break;

    case Atom::SectionHeadingRight:
        inSectionHeading = false;
        break;

    case Atom::SidebarLeft:
    case Atom::SidebarRight:
        break;

    case Atom::String:
        writer.writeCharacters(atom->string());
        break;

    case Atom::TableLeft:
        writer.writeStartElement("table");

        numTableRows = 0;
        atom = atom->next();
        while (atom && atom->type() != Atom::TableRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // table
        break;

    case Atom::TableRight:
        break;

    case Atom::TableHeaderLeft:
        writer.writeStartElement("header");

        atom = atom->next();
        while (atom && atom->type() != Atom::TableHeaderRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // header
        break;

    case Atom::TableHeaderRight:
        break;

    case Atom::TableRowLeft:
        writer.writeStartElement("row");

        atom = atom->next();
        while (atom && atom->type() != Atom::TableRowRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // row
        break;

    case Atom::TableRowRight:
        break;

    case Atom::TableItemLeft:
        writer.writeStartElement("item");

        atom = atom->next();
        while (atom && atom->type() != Atom::TableItemRight)
            atom = addAtomElements(writer, atom, relative, marker);

        writer.writeEndElement(); // item
        break;

    case Atom::TableItemRight:
        break;

    case Atom::TableOfContents:
        writer.writeStartElement("tableofcontents");
/*        {
            int numColumns = 1;
            const Node *node = relative;

            Doc::SectioningUnit sectioningUnit = Doc::Section4;
            QStringList params = atom->string().split(",");
            QString columnText = params.at(0);
            QStringList pieces = columnText.split(" ", QString::SkipEmptyParts);
            if (pieces.size() >= 2) {
                columnText = pieces.at(0);
                pieces.pop_front();
                QString path = pieces.join(" ").trimmed();
                node = findNodeForTarget(path, relative, marker, atom);
            }

            if (params.size() == 2) {
                numColumns = qMax(columnText.toInt(), numColumns);
                sectioningUnit = (Doc::SectioningUnit)params.at(1).toInt();
            }

            if (node)
                generateTableOfContents(node, marker, sectioningUnit, numColumns,
                                        relative);
        }*/
        writer.writeEndElement(); // tableofcontents
        break;

    case Atom::Target:
        writer.writeStartElement("target");
        writer.writeAttribute("name", Doc::canonicalTitle(atom->string()));
        writer.writeEndElement(); // target
        break;

    case Atom::UnhandledFormat:
    case Atom::UnknownCommand:
        writer.writeCharacters(atom->typeString());
        break;
    default:
        break;
    }

    if (atom)
        return atom->next();
    
    return 0;
}
/*
        QDomElement atomElement = document.createElement(atom->typeString().toLower());
        QDomText atomValue = document.createTextNode(atom->string());
        atomElement.appendChild(atomValue);
        descriptionElement.appendChild(atomElement);
*/
