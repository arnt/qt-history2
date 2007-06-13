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

#include "layout_propertysheet.h"

// sdk
#include <QtDesigner/QExtensionManager>

// shared
#include <qlayout_widget_p.h>
#include <QtGui/QStyle>

static const char *leftMargin = "leftMargin";
static const char *topMargin = "topMargin";
static const char *rightMargin = "rightMargin";
static const char *bottomMargin = "bottomMargin";
static const char *horizontalSpacing = "horizontalSpacing";
static const char *verticalSpacing = "verticalSpacing";
static const char *spacing = "spacing";
static const char *margin = "margin";

namespace qdesigner_internal {

LayoutPropertySheet::LayoutPropertySheet(QLayout *object, QObject *parent)
    : QDesignerPropertySheet(object, parent), m_layout(object)
{
    const QString layoutGroup = QLatin1String("Layout");
    int pindex = count();
    createFakeProperty(QLatin1String(leftMargin), 0);
    setPropertyGroup(pindex, layoutGroup);

    pindex = count();
    createFakeProperty(QLatin1String(topMargin), 0);
    setPropertyGroup(pindex, layoutGroup);

    pindex = count();
    createFakeProperty(QLatin1String(rightMargin), 0);
    setPropertyGroup(pindex, layoutGroup);

    pindex = count();
    createFakeProperty(QLatin1String(bottomMargin), 0);
    setPropertyGroup(pindex, layoutGroup);

    QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout);
    if (grid) {
        pindex = count();
        createFakeProperty(QLatin1String(horizontalSpacing), 0);
        setPropertyGroup(pindex, layoutGroup);

        pindex = count();
        createFakeProperty(QLatin1String(verticalSpacing), 0);
        setPropertyGroup(pindex, layoutGroup);

        setAttribute(indexOf(QLatin1String(spacing)), true);
    } else {
    }

    setAttribute(indexOf(QLatin1String(margin)), true);
}

LayoutPropertySheet::~LayoutPropertySheet()
{
}

void LayoutPropertySheet::setProperty(int index, const QVariant &value)
{
    const QString pname = propertyName(index);
    QLayoutWidget *lw = qobject_cast<QLayoutWidget *>(m_layout->parent());
    if (lw) {
        if (pname == QLatin1String(margin)) {
            const int v = value.toInt();
            lw->setLayoutLeftMargin(v);
            lw->setLayoutTopMargin(v);
            lw->setLayoutRightMargin(v);
            lw->setLayoutBottomMargin(v);
            return;
        }
        if (pname == QLatin1String(leftMargin)) {
            lw->setLayoutLeftMargin(value.toInt());
            return;
        }
        if (pname == QLatin1String(topMargin)) {
            lw->setLayoutTopMargin(value.toInt());
            return;
        }
        if (pname == QLatin1String(rightMargin)) {
            lw->setLayoutRightMargin(value.toInt());
            return;
        }
        if (pname == QLatin1String(bottomMargin)) {
            lw->setLayoutBottomMargin(value.toInt());
            return;
        }
    }
    QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout);
    int left, top, right, bottom;
    m_layout->getContentsMargins(&left, &top, &right, &bottom);
    if (pname == QLatin1String(leftMargin))
        m_layout->setContentsMargins(value.toInt(), top, right, bottom);
    else if (pname == QLatin1String(topMargin))
        m_layout->setContentsMargins(left, value.toInt(), right, bottom);
    else if (pname == QLatin1String(rightMargin))
        m_layout->setContentsMargins(left, top, value.toInt(), bottom);
    else if (pname == QLatin1String(bottomMargin))
        m_layout->setContentsMargins(left, top, right, value.toInt());
    else if (grid && pname == QLatin1String(horizontalSpacing))
        grid->setHorizontalSpacing(value.toInt());
    else if (grid && pname == QLatin1String(verticalSpacing))
        grid->setVerticalSpacing(value.toInt());
    else
        QDesignerPropertySheet::setProperty(index, value);
}

QVariant LayoutPropertySheet::property(int index) const
{
    const QString pname = propertyName(index);
    QLayoutWidget *lw = qobject_cast<QLayoutWidget *>(m_layout->parent());
    if (lw) {
        if (pname == QLatin1String(leftMargin))
            return lw->layoutLeftMargin();
        if (pname == QLatin1String(topMargin))
            return lw->layoutTopMargin();
        if (pname == QLatin1String(rightMargin))
            return lw->layoutRightMargin();
        if (pname == QLatin1String(bottomMargin))
            return lw->layoutBottomMargin();
    }
    QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout);
    int left, top, right, bottom;
    m_layout->getContentsMargins(&left, &top, &right, &bottom);
    if (pname == QLatin1String(leftMargin))
        return left;
    if (pname == QLatin1String(topMargin))
        return top;
    if (pname == QLatin1String(rightMargin))
        return right;
    if (pname == QLatin1String(bottomMargin))
        return bottom;
    if (grid && pname == QLatin1String(horizontalSpacing))
        return grid->horizontalSpacing();
    if (grid && pname == QLatin1String(verticalSpacing))
        return grid->verticalSpacing();
    return QDesignerPropertySheet::property(index);
}

bool LayoutPropertySheet::reset(int index)
{
    const QString pname = propertyName(index);
    int left, top, right, bottom;
    m_layout->getContentsMargins(&left, &top, &right, &bottom);
    if (pname == QLatin1String(leftMargin))
        m_layout->setContentsMargins(-1, top, right, bottom);
    else if (pname == QLatin1String(topMargin))
        m_layout->setContentsMargins(left, -1, right, bottom);
    else if (pname == QLatin1String(rightMargin))
        m_layout->setContentsMargins(left, top, -1, bottom);
    else if (pname == QLatin1String(bottomMargin))
        m_layout->setContentsMargins(left, top, right, -1);
    else
        return QDesignerPropertySheet::reset(index);
    return true;
}

void LayoutPropertySheet::setChanged(int index, bool changed)
{
    QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout);
    const QString pname = propertyName(index);
    if (pname == QLatin1String(margin)) {
        setChanged(indexOf(QLatin1String(leftMargin)), changed);
        setChanged(indexOf(QLatin1String(topMargin)), changed);
        setChanged(indexOf(QLatin1String(rightMargin)), changed);
        setChanged(indexOf(QLatin1String(bottomMargin)), changed);
    }
    if (pname == QLatin1String(spacing) && grid) {
        setChanged(indexOf(QLatin1String(horizontalSpacing)), changed);
        setChanged(indexOf(QLatin1String(verticalSpacing)), changed);
    }
    QDesignerPropertySheet::setChanged(index, changed);
}
}
