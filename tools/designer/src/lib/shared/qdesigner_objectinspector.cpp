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

#include "qdesigner_objectinspector_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDesignerObjectInspector::QDesignerObjectInspector(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerObjectInspectorInterface(parent, flags)
{
}

void QDesignerObjectInspector::mainContainerChanged()
{
}

void Selection::clear()
{
    m_cursorSelection.clear();
    m_selectedObjects.clear();
}

bool Selection::empty() const
{
    return m_cursorSelection.empty() && m_selectedObjects.empty();
}

Selection::ObjectList Selection::selection() const
{
    ObjectList rc(m_selectedObjects);
    foreach (QObject* o, m_cursorSelection) {
        rc.push_back(o);
    }
    return rc;
}
}

QT_END_NAMESPACE
