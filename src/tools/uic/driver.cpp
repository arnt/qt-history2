/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "driver.h"
#include "uic.h"
#include "ui4.h"

#include <qregexp.h>
#include <qfileinfo.h>
#include <qdebug.h>

Driver::Driver()
{
    m_output = 0;
}

Driver::~Driver()
{
}

QString Driver::findOrInsertWidget(DomWidget *ui_widget)
{
    if (!m_widgets.contains(ui_widget))
        m_widgets.insert(ui_widget, unique(ui_widget->attributeName(), ui_widget->attributeClass()));

    return m_widgets.value(ui_widget);
}

QString Driver::findOrInsertSpacer(DomSpacer *ui_spacer)
{
    if (!m_spacers.contains(ui_spacer))
        m_spacers.insert(ui_spacer, unique(QString::null, QLatin1String("QSpacerItem")));

    return m_spacers.value(ui_spacer);
}

QString Driver::findOrInsertLayout(DomLayout *ui_layout)
{
    if (!m_layouts.contains(ui_layout))
        m_layouts.insert(ui_layout, unique(QString::null, ui_layout->attributeClass()));

    return m_layouts.value(ui_layout);
}

QString Driver::findOrInsertLayoutItem(DomLayoutItem *ui_layoutItem)
{
    switch (ui_layoutItem->kind()) {
        case DomLayoutItem::Widget:
            return findOrInsertWidget(ui_layoutItem->elementWidget());
        case DomLayoutItem::Spacer:
            return findOrInsertSpacer(ui_layoutItem->elementSpacer());
        case DomLayoutItem::Layout:
            return findOrInsertLayout(ui_layoutItem->elementLayout());
        case DomLayoutItem::Unknown:
            break;
    }

    Q_ASSERT( 0 );

    return QString::null;
}

QString Driver::findOrInsertActionGroup(DomActionGroup *ui_group)
{
    if (!m_actionGroups.contains(ui_group))
        m_actionGroups.insert(ui_group, unique(ui_group->attributeName(), "QActionGroup"));

    return m_actionGroups.value(ui_group);
}

QString Driver::findOrInsertAction(DomAction *ui_action)
{
    if (!m_actions.contains(ui_action))
        m_actions.insert(ui_action, unique(ui_action->attributeName(), "QAction"));

    return m_actions.value(ui_action);
}

QString Driver::findOrInsertName(const QString &name)
{
    QString n = unique(name);
    m_nameRepository.insert(n, true);
    return n;
}


QString Driver::unique(const QString &instanceName, const QString &className)
{
    if (instanceName.size()) {
        int id = 1;
        QString name = instanceName;
        while (true) {
            if (!m_nameRepository.contains(name))
                break;

            name = instanceName + QString::number(id++);
        }

        m_nameRepository.insert(name, true);
        return name;
    } else if (className.size()) {
        QString name = unique(qtify(className));
        m_nameRepository.insert(name, true);
        return name;
    }

    QString name = unique("var");
    m_nameRepository.insert(name, true);
    return name;
}

QString Driver::qtify(const QString &name)
{
    QString qname = name;
    if (qname.at(0) == 'Q' || qname.at(0) == 'K')
        qname = qname.mid(1);

    int i=0;
    while (i < qname.length()) {
        if (qname.at(i).toLower() != qname.at(i))
            qname[i] = qname.at(i).toLower();
        else
            break;

        ++i;
    }

    return qname;
}

QString Driver::headerFileName(const QString &fileName)
{
    if (fileName.isEmpty())
        return headerFileName(QLatin1String("noname"));

    QFileInfo info(fileName);
    return info.baseName().toUpper() + "_H";
}

bool Driver::uic(const QString &fileName, DomUI *ui, QTextStream *out)
{
    m_option.inputFile = fileName;

    QTextStream *oldOutput = m_output;
    bool deleteOutput = false;

    if (out) {
        m_output = out;
    } else {
        m_output = new QTextStream(stdout, IO_WriteOnly);
        deleteOutput = true;
    }

    Uic tool(this);
    bool rtn = tool.write(ui);

    if (deleteOutput)
        delete m_output;

    m_output = oldOutput;

    return rtn;
}

bool Driver::uic(const QString &fileName, QTextStream *out)
{
    QFile f;
    if (fileName.isEmpty())
        f.open(IO_ReadOnly, stdin);
    else {
        f.setName(fileName);
        if (!f.open(IO_ReadOnly))
            return false;
    }

    m_option.inputFile = fileName;

    QTextStream *oldOutput = m_output;
    bool deleteOutput = false;

    if (out) {
        m_output = out;
    } else {
        m_output = new QTextStream(stdout, IO_WriteOnly);
        deleteOutput = true;
    }

    Uic tool(this);
    bool rtn = tool.write(&f);
    f.close();

    if (deleteOutput)
        delete m_output;

    m_output = oldOutput;

    return rtn;
}

void Driver::reset()
{
    Q_ASSERT( m_output == 0 );

    m_option = Option();
    m_output = 0;
    m_problems.clear();

    QStringList m_problems;

    m_widgets.clear();
    m_spacers.clear();
    m_layouts.clear();
    m_actionGroups.clear();
    m_actions.clear();
    m_nameRepository.clear();
    m_pixmaps.clear();
}

void Driver::insertPixmap(const QString &pixmap)
{
    m_pixmaps.insert(pixmap, true);
}

bool Driver::containsPixmap(const QString &pixmap) const
{
    return m_pixmaps.contains(pixmap);
}
