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

#include "qmdiarea_container.h"

#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>

namespace qdesigner_internal {

QMdiAreaContainer::QMdiAreaContainer(QMdiArea *widget, QObject *parent)
    : QObject(parent),
      m_mdiArea(widget)
{
    Q_ASSERT(m_mdiArea->subWindowList(QMdiArea::CreationOrder).isEmpty());
}

int QMdiAreaContainer::count() const
{
    return m_mdiArea->subWindowList(QMdiArea::CreationOrder).count();
}

QWidget *QMdiAreaContainer::widget(int index) const
{
    return m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index)->widget();
}

int QMdiAreaContainer::currentIndex() const
{
    return m_mdiArea->subWindowList(QMdiArea::CreationOrder).indexOf(m_mdiArea->activeSubWindow());
}

void QMdiAreaContainer::setCurrentIndex(int index)
{
    m_mdiArea->setActiveSubWindow(m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index));
}

void QMdiAreaContainer::addWidget(QWidget *widget)
{
    QMdiSubWindow *frame = m_mdiArea->addSubWindow(widget, Qt::Window);
    frame->show();
}

void QMdiAreaContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);

    addWidget(widget);
}

void QMdiAreaContainer::remove(int index)
{
    Q_UNUSED(index);
}

QMdiAreaContainerFactory::QMdiAreaContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QMdiAreaContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (QMdiArea *w = qobject_cast<QMdiArea*>(object))
        return new QMdiAreaContainer(w, parent);

    return 0;
}
}
