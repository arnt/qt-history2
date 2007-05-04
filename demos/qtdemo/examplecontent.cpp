/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "examplecontent.h"
#include "colors.h"
#include "menumanager.h"
#include "imageitem.h"
#include "headingitem.h"

ExampleContent::ExampleContent(const QString &name, QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    this->name = name;
    this->heading = 0;
    this->description = 0;
    this->screenshot = 0;
}

void ExampleContent::prepare()
{
    if (!this->prepared){
        this->prepared = true;
        this->createContent();
    }
}

void ExampleContent::animationStopped(int id)
{
    if (id == DemoItemAnimation::ANIM_OUT){
        // Free up some memory:
        delete this->heading;
        delete this->description;
        delete this->screenshot;
        this->heading = 0;
        this->description = 0;
        this->screenshot = 0;
        this->prepared = false;
    }
}

QString ExampleContent::loadDescription()
{
    QDomDocument exampleDoc;
    QFile exampleFile(MenuManager::instance()->info[this->name]["docfile"]);
    exampleDoc.setContent(&exampleFile);

    QDomNodeList paragraphs = exampleDoc.elementsByTagName("p");
    if (paragraphs.length() < 1 && Colors::verbose)
        qDebug() << "- ExampleContent::loadDescription(): Could not load description:" << MenuManager::instance()->info[this->name]["docfile"];
    QString description = Colors::contentColor + QLatin1String("Could not load description. Ensure that the documentation for Qt is built.");
    for (int p = 0; p < int(paragraphs.length()); ++p) {
        description = this->extractTextFromParagraph(paragraphs.item(p));
        if (this->isSummary(description)) {
            break;
        }
    }
    return Colors::contentColor + description;
}

bool ExampleContent::isSummary(const QString &text)
{
    return (text.indexOf(QRegExp(QString(
            "((The|This) )?(%1 )?.*(example|demo)").arg(this->name), Qt::CaseInsensitive)) != -1);
}

QString ExampleContent::extractTextFromParagraph(const QDomNode &parentNode)
{
    QString description;
    QDomNode node = parentNode.firstChild();

    while (!node.isNull()) {
        QString beginTag;
        QString endTag;
        if (node.isText())
            description += Colors::contentColor + node.nodeValue();
        else if (node.hasChildNodes()) {
            if (node.nodeName() == "b") {
                beginTag = "<b>";
                endTag = "</b>";
            } else if (node.nodeName() == "a") {
                beginTag = Colors::contentColor;
                endTag = "</font>";
            } else if (node.nodeName() == "i") {
                beginTag = "<i>";
                endTag = "</i>";
            } else if (node.nodeName() == "tt") {
                beginTag = "<tt>";
                endTag = "</tt>";
            }
            description += beginTag + this->extractTextFromParagraph(node) + endTag;
        }
        node = node.nextSibling();
    }

    return description;
}

void ExampleContent::createContent()
{
    // Create the items:
    this->heading = new HeadingItem(this->name, this->scene(), this);
    this->description = new DemoTextItem(this->loadDescription(), Colors::contentFont(),
                                            Colors::heading, 500, this->scene(), this);
    int imgHeight = 340 - int(this->description->boundingRect().height()) + 50;
    this->screenshot = new ImageItem(MenuManager::instance()->info[this->name]["imgfile"],
                                            550, imgHeight, this->scene(), this);
    // Place the items on screen:
    this->heading->setPos(0, 3);
    this->description->setPos(0, this->heading->pos().y() + this->heading->boundingRect().height() + 10);
    this->screenshot->setPos(0, this->description->pos().y() + this->description->boundingRect().height() + 10);
}

QRectF ExampleContent::boundingRect() const
{
    return QRectF(0, 0, 500, 100);
}


