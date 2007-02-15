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
TRANSLATOR qdesigner_internal::FormWindowBase
*/

#include "formwindowbase_p.h"
#include <QtCore/qdebug.h>

namespace qdesigner_internal {

Grid FormWindowBase::m_defaultGrid;

FormWindowBase::FormWindowBase(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerFormWindowInterface(parent, flags),
    m_feature(DefaultFeature),
    m_grid(m_defaultGrid)
{
    syncGridFeature();
}

QVariantMap FormWindowBase::formData()
{
    QVariantMap rc;
    m_grid.addToVariantMap(rc);
    return rc;
}

void FormWindowBase::setFormData(const QVariantMap &vm)
{
    m_grid.fromVariantMap(vm);
}

QPoint FormWindowBase::grid() const
{
    return QPoint(m_grid.deltaX(), m_grid.deltaY());
}

void FormWindowBase::setGrid(const QPoint &grid)
{
    m_grid.setDeltaX(grid.x());
    m_grid.setDeltaY(grid.y());
}

bool FormWindowBase::hasFeature(Feature f) const
{
    return f & m_feature;
}

static void recursiveUpdate(QWidget *w)
{
    w->update();

    const QObjectList &l = w->children();
    const QObjectList::const_iterator cend = l.constEnd();
    for (QObjectList::const_iterator it = l.constBegin(); it != cend; ++it) {
        if (QWidget *w = qobject_cast<QWidget*>(*it))
            recursiveUpdate(w);
    }
}

void FormWindowBase::setFeatures(Feature f)
{
    m_feature = f;
    const bool enableGrid = f & GridFeature;
    m_grid.setVisible(enableGrid);
    m_grid.setSnapX(enableGrid);
    m_grid.setSnapY(enableGrid);
    emit featureChanged(f);
    recursiveUpdate(this);
}

FormWindowBase::Feature FormWindowBase::features() const
{
    return m_feature;
}

bool FormWindowBase::gridVisible() const
{
    return m_grid.visible() && currentTool() == 0;
}

void FormWindowBase::syncGridFeature()
{
    if (m_grid.snapX() || m_grid.snapY())
        m_feature |= GridFeature;
    else
        m_feature &= ~GridFeature;
}

void FormWindowBase::setDesignerGrid(const  Grid& grid)
{
    m_grid = grid;
    syncGridFeature();
    recursiveUpdate(this);
}

void FormWindowBase::setDefaultDesignerGrid(const  Grid& grid)
{
    m_defaultGrid = grid;
}
} // namespace
