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
#include <QtCore/QHash>
#include <QtCore/QDebug>

#define USE_LAYOUT_SIZE_CONSTRAINT

static const char *leftMargin = "leftMargin";
static const char *topMargin = "topMargin";
static const char *rightMargin = "rightMargin";
static const char *bottomMargin = "bottomMargin";
static const char *horizontalSpacing = "horizontalSpacing";
static const char *verticalSpacing = "verticalSpacing";
static const char *spacing = "spacing";
static const char *margin = "margin";
static const char *sizeConstraint = "sizeConstraint";

namespace {
    enum LayoutPropertyType {
        LayoutPropertyNone,
        LayoutPropertyMargin, // Deprecated
        LayoutPropertyLeftMargin,
        LayoutPropertyTopMargin,
        LayoutPropertyRightMargin,
        LayoutPropertyBottomMargin,
        LayoutPropertySpacing,
        LayoutPropertyHorizontalSpacing,
        LayoutPropertyVerticalSpacing,
        LayoutPropertySizeConstraint
    };
}

// Quick lookup by name
static LayoutPropertyType  layoutPropertyType(const QString &name)
{
    static QHash<QString, LayoutPropertyType> namePropertyMap;
    if (namePropertyMap.empty()) {
        namePropertyMap.insert(QLatin1String(leftMargin), LayoutPropertyLeftMargin);
        namePropertyMap.insert(QLatin1String(topMargin), LayoutPropertyTopMargin);
        namePropertyMap.insert(QLatin1String(rightMargin), LayoutPropertyRightMargin);
        namePropertyMap.insert(QLatin1String(bottomMargin), LayoutPropertyBottomMargin);
        namePropertyMap.insert(QLatin1String(horizontalSpacing), LayoutPropertyHorizontalSpacing);
        namePropertyMap.insert(QLatin1String(verticalSpacing), LayoutPropertyVerticalSpacing);
        namePropertyMap.insert(QLatin1String(spacing), LayoutPropertySpacing);
        namePropertyMap.insert(QLatin1String(margin), LayoutPropertyMargin);
        namePropertyMap.insert(QLatin1String(sizeConstraint), LayoutPropertySizeConstraint);
    }
    return namePropertyMap.value(name, LayoutPropertyNone);
}

// return the layout margin if it is  margin
static int getLayoutMargin(const QLayout *l, LayoutPropertyType type)
{
    int left, top, right, bottom;
    l->getContentsMargins(&left, &top, &right, &bottom);
    switch (type) {
    case LayoutPropertyLeftMargin:
        return left;
    case LayoutPropertyTopMargin:
        return top;
    case LayoutPropertyRightMargin:
        return right;
    case LayoutPropertyBottomMargin:
        return bottom;
    default:
        Q_ASSERT(0);
        break;
    }
    return 0;
}

// return the layout margin if it is  margin
static void setLayoutMargin(QLayout *l, LayoutPropertyType type, int margin)
{
    int left, top, right, bottom;
    l->getContentsMargins(&left, &top, &right, &bottom);
    switch (type) {
    case LayoutPropertyLeftMargin:
        left = margin;
        break;
    case LayoutPropertyTopMargin:
        top = margin;
        break;
    case LayoutPropertyRightMargin:
        right = margin;
        break;
    case LayoutPropertyBottomMargin:
        bottom = margin;
        break;
    default:
        Q_ASSERT(0);
        break;
    }
    l->setContentsMargins(left, top, right, bottom);
}

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

    if (LayoutProperties::visibleProperties(m_layout) & LayoutProperties::HorizSpacingProperty) {
        pindex = count();
        createFakeProperty(QLatin1String(horizontalSpacing), 0);
        setPropertyGroup(pindex, layoutGroup);

        pindex = count();
        createFakeProperty(QLatin1String(verticalSpacing), 0);
        setPropertyGroup(pindex, layoutGroup);

        setAttribute(indexOf(QLatin1String(spacing)), true);
    }

    setAttribute(indexOf(QLatin1String(margin)), true);

#ifdef USE_LAYOUT_SIZE_CONSTRAINT
    // SizeConstraint cannot possibly be handled as a real property
    // as it affects the layout parent widget and thus
    // conflicts with Designer's special layout widget.
    // It will take effect on the preview only.
    pindex = count();
    createFakeProperty(QLatin1String(sizeConstraint));
    setPropertyGroup(pindex, layoutGroup);
#endif
}

LayoutPropertySheet::~LayoutPropertySheet()
{
}

void LayoutPropertySheet::setProperty(int index, const QVariant &value)
{
    const LayoutPropertyType type = layoutPropertyType(propertyName(index));
    if (QLayoutWidget *lw = qobject_cast<QLayoutWidget *>(m_layout->parent())) {
        switch (type) {
        case LayoutPropertyLeftMargin:
            lw->setLayoutLeftMargin(value.toInt());
            return;
        case LayoutPropertyTopMargin:
            lw->setLayoutTopMargin(value.toInt());
            return;
        case LayoutPropertyRightMargin:
            lw->setLayoutRightMargin(value.toInt());
            return;
        case LayoutPropertyBottomMargin:
            lw->setLayoutBottomMargin(value.toInt());
            return;
        case LayoutPropertyMargin: {
            const int v = value.toInt();
            lw->setLayoutLeftMargin(v);
            lw->setLayoutTopMargin(v);
            lw->setLayoutRightMargin(v);
            lw->setLayoutBottomMargin(v);
        }
            return;
        default:
            break;
        }
    }
    switch (type) {
    case LayoutPropertyLeftMargin:
    case LayoutPropertyTopMargin:
    case LayoutPropertyRightMargin:
    case LayoutPropertyBottomMargin:
        setLayoutMargin(m_layout, type, value.toInt());
        return;
    case LayoutPropertyHorizontalSpacing:
        if (QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout)) {
            grid->setHorizontalSpacing(value.toInt());
            return;
        }
        break;
    case LayoutPropertyVerticalSpacing:
        if (QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout)) {
            grid->setVerticalSpacing(value.toInt());
            return;
        }
        break;
    default:
        break;
    }
    QDesignerPropertySheet::setProperty(index, value);
}

QVariant LayoutPropertySheet::property(int index) const
{
    const LayoutPropertyType type = layoutPropertyType(propertyName(index));
    if (const QLayoutWidget *lw = qobject_cast<QLayoutWidget *>(m_layout->parent())) {
        switch (type) {
        case LayoutPropertyLeftMargin:
            return lw->layoutLeftMargin();
        case LayoutPropertyTopMargin:
            return lw->layoutTopMargin();
        case LayoutPropertyRightMargin:
            return lw->layoutRightMargin();
        case LayoutPropertyBottomMargin:
             return lw->layoutBottomMargin();
        default:
            break;
        }
    }
    switch (type) {
    case LayoutPropertyLeftMargin:
    case LayoutPropertyTopMargin:
    case LayoutPropertyRightMargin:
    case LayoutPropertyBottomMargin:
        return getLayoutMargin(m_layout, type);
    case LayoutPropertyHorizontalSpacing:
        if (const QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout))
            return grid->horizontalSpacing();
        break;
    case LayoutPropertyVerticalSpacing:
        if (const QGridLayout *grid = qobject_cast<QGridLayout *>(m_layout))
            return grid->verticalSpacing();
    default:
        break;
    }
    return QDesignerPropertySheet::property(index);
}

bool LayoutPropertySheet::reset(int index)
{
    int left, top, right, bottom;
    m_layout->getContentsMargins(&left, &top, &right, &bottom);
    const LayoutPropertyType type = layoutPropertyType(propertyName(index));
    bool rc = true;
    switch (type) {
    case LayoutPropertyLeftMargin:
        m_layout->setContentsMargins(-1, top, right, bottom);
        break;
    case LayoutPropertyTopMargin:
        m_layout->setContentsMargins(left, -1, right, bottom);
        break;
    case LayoutPropertyRightMargin:
        m_layout->setContentsMargins(left, top, -1, bottom);
        break;
    case LayoutPropertyBottomMargin:
        m_layout->setContentsMargins(left, top, right, -1);
        break;
    default:
        rc = QDesignerPropertySheet::reset(index);
        break;
    }
    return rc;
}

void LayoutPropertySheet::setChanged(int index, bool changed)
{
    const LayoutPropertyType type = layoutPropertyType(propertyName(index));
    switch (type) {
    case LayoutPropertySpacing:
        if (LayoutProperties::visibleProperties(m_layout) & LayoutProperties::HorizSpacingProperty) {
            setChanged(indexOf(QLatin1String(horizontalSpacing)), changed);
            setChanged(indexOf(QLatin1String(verticalSpacing)), changed);
        }
        break;
    case LayoutPropertyMargin:
        setChanged(indexOf(QLatin1String(leftMargin)), changed);
        setChanged(indexOf(QLatin1String(topMargin)), changed);
        setChanged(indexOf(QLatin1String(rightMargin)), changed);
        setChanged(indexOf(QLatin1String(bottomMargin)), changed);
        break;
    default:
        break;
    }
    QDesignerPropertySheet::setChanged(index, changed);
}
}
