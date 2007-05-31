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
#include "connectionedit_p.h"
#include "qdesigner_command_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qdebug.h>
#include <QtGui/QMenu>

namespace qdesigner_internal {

Grid FormWindowBase::m_defaultGrid;

FormWindowBase::FormWindowBase(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerFormWindowInterface(parent, flags),
    m_feature(DefaultFeature),
    m_grid(m_defaultGrid),
    m_hasFormGrid(false)
{
    syncGridFeature();
}

QVariantMap FormWindowBase::formData()
{
    QVariantMap rc;
    if (m_hasFormGrid)
        m_grid.addToVariantMap(rc, true);
    return rc;
}

void FormWindowBase::setFormData(const QVariantMap &vm)
{
    Grid formGrid;
    m_hasFormGrid = formGrid.fromVariantMap(vm);
    if (m_hasFormGrid)
         m_grid = formGrid;
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

QMenu *FormWindowBase::initializePopupMenu(QWidget * /*managedWidget*/)
{
    return 0;
}

QWidget *FormWindowBase::widgetUnderMouse(const QPoint &formPos, WidgetUnderMouseMode /* wum */)
{
    // widget_under_mouse might be some temporary thing like the dropLine. We need
    // the actual widget that's part of the edited GUI.
    QWidget *rc = widgetAt(formPos);
    if (!rc || qobject_cast<ConnectionEdit*>(rc))
        return 0;

    if (QWidget *container = findContainer(rc, false))
        if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), container))
            rc = c->widget(c->currentIndex());

    return rc;
}

void FormWindowBase::deleteWidgetList(const QWidgetList &widget_list)
{
    switch (widget_list.size()) {
    case 0:
        break;
    case 1: {
        commandHistory()->beginMacro(tr("Delete '%1'").arg(widget_list.front()->objectName()));
        emit widgetRemoved(widget_list.front());
        DeleteWidgetCommand *cmd = new DeleteWidgetCommand(this);
        cmd->init(widget_list.front());
        commandHistory()->push(cmd);
        commandHistory()->endMacro();
    }
        break;
    default:
        commandHistory()->beginMacro(tr("Delete"));
        foreach (QWidget *w, widget_list) {
            emit widgetRemoved(w);
            DeleteWidgetCommand *cmd = new DeleteWidgetCommand(this);
            cmd->init(w);
            commandHistory()->push(cmd);
        }
        commandHistory()->endMacro();
        break;
    }
}
}
