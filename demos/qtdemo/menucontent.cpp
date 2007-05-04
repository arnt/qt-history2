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
    this->heading = 0;
    this->description1 = 0;
    this->description2 = 0;

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

void MenuContentItem::animationStopped(int id)
{
    if (this->name == QLatin1String("Qt Examples and Demos"))
        return; // Optimization hack.
        
    if (id == DemoItemAnimation::ANIM_OUT){
        // Free up some memory:
        delete this->heading;
        delete this->description1;
        delete this->description2;
        this->heading = 0;
        this->description1 = 0;
        this->description2 = 0;
        this->prepared = false;
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
    // Create the items:
    this->heading = new HeadingItem(this->name, this->scene(), this);
    QString para1 = this->loadDescription(0, 1);
    if (para1.isEmpty())
        para1 = Colors::contentColor + QLatin1String("Could not load description. Ensure that the documentation for Qt is built.");
    QColor bgcolor = Colors::sceneBg1.darker(200);
    bgcolor.setAlpha(100);
    this->description1 = new DemoTextItem(para1, Colors::contentFont(), Colors::heading, 500, this->scene(), this, DemoTextItem::STATIC_TEXT);
    this->description2 = new DemoTextItem(this->loadDescription(1, 2), Colors::contentFont(), Colors::heading, 250, this->scene(), this, DemoTextItem::STATIC_TEXT);

    // Place the items on screen:
    this->heading->setPos(0, 3);
    this->description1->setPos(0, this->heading->pos().y() + this->heading->boundingRect().height() + 10);
    this->description2->setPos(0, this->description1->pos().y() + this->description1->boundingRect().height() + 15);
}

QRectF MenuContentItem::boundingRect() const
{
    return QRectF(0, 0, 500, 350);
}


