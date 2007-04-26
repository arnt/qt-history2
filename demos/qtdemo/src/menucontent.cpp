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

#include "menucontent.h"
#include "colors.h"
#include "menumanager.h"
#include "demotextitem.h"
#include "headingitem.h"

MenuContentItem::MenuContentItem(const QDomElement &el, QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    this->name = el.attribute("name");

    if (el.tagName() == "demos")
        this->readmePath = QLibraryInfo::location(QLibraryInfo::DemosPath) + "/README";
    else
        this->readmePath = QLibraryInfo::location(QLibraryInfo::ExamplesPath) + "/" + el.attribute("dirname") + "/README";

}

void MenuContentItem::prepare()
{
    if (!this->prepared){
        this->prepared= true;
        this->createContent();
    }
}

QString MenuContentItem::loadDescription(int startPara, int nrPara)
{
    QString result;
    QFile readme(this->readmePath);
    if (!readme.open(QFile::ReadOnly)){
        if (Colors::verbose)
            qDebug() << "- MenuContentItem::loadDescription: Could not load:" << this->readmePath;
        return "";
    }

    QTextStream in(&readme);
    // Skip a certain number of paragraphs:
    while (startPara)
        if (in.readLine().isEmpty()) --startPara;

    // Read in the number of wanted paragraphs:
    QString line = in.readLine();
    do {
        result += line + " ";
        line = in.readLine();
        if (line.isEmpty()){
            --nrPara;
            line = "<br><br>" + in.readLine();
        }
    } while (nrPara && !in.atEnd());

    return Colors::contentColor + result;
}

void MenuContentItem::createContent()
{
    HeadingItem *heading = new HeadingItem(this->name, this->scene(), this);
    QString para1 = this->loadDescription(0, 1);
    if (para1.isEmpty())
        para1 = Colors::contentColor + QLatin1String("Could not load description. Ensure that the documentation for Qt is built.");
    QColor bgcolor = Colors::sceneBg1.darker(200);
    bgcolor.setAlpha(100);
    DemoTextItem *s1 = new DemoTextItem(para1, Colors::contentFont(), Colors::heading, 500, this->scene(), this, DemoTextItem::STATIC_TEXT);
    DemoTextItem *s2 = new DemoTextItem(this->loadDescription(1, 2), Colors::contentFont(), Colors::heading, 250, this->scene(), this, DemoTextItem::STATIC_TEXT);
    heading->setPos(0, 3);
    s1->setPos(0, heading->pos().y() + heading->boundingRect().height() + 10);
    s2->setPos(0, s1->pos().y() + s1->boundingRect().height() + 15);
}

QRectF MenuContentItem::boundingRect() const
{
    return QRectF(0, 0, 500, 350);
}


