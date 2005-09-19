/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_menubar_p.h"
#include "qdesigner_toolbar_p.h"
#include "actionrepository_p.h"
#include "actionprovider_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

QDesignerMenuBar::QDesignerMenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    m_blockSentinelChecker = false;
    m_sentinel = 0;

    setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true); // ### fake

    m_sentinel = new SentinelAction(this);       // ### use a special widget as indicator
    addAction(m_sentinel);

    m_sentinelChecker = new QTimer(this);
    connect(m_sentinelChecker, SIGNAL(timeout()), this, SLOT(slotCheckSentinel()));

    qApp->installEventFilter(this);
}

QDesignerMenuBar::~QDesignerMenuBar()
{
}

bool QDesignerMenuBar::handleEvent(QWidget *widget, QEvent *event)
{
    if (!formWindow())
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::MouseButtonPress:
            return handleMousePressEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleMouseReleaseEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return handleMouseMoveEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::ContextMenu:
            return handleContextMenuEvent(widget, static_cast<QContextMenuEvent*>(event));
    }

    return true;
}

bool QDesignerMenuBar::handleMousePressEvent(QWidget *, QMouseEvent *)
{
    return false;
}

bool QDesignerMenuBar::handleMouseReleaseEvent(QWidget *, QMouseEvent *)
{
    return false;
}

bool QDesignerMenuBar::handleMouseMoveEvent(QWidget *, QMouseEvent *)
{
    return false;
}

bool QDesignerMenuBar::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
{
    event->accept();

    int index = findAction(mapFromGlobal(event->globalPos()));
    QAction *action = actions().at(index);
    if (action == actions().last())
        return true;

    QMenu menu(0);
    QAction *a = menu.addAction(tr("Remove action '%1'").arg(action->objectName()));
    QVariant itemData;
    qVariantSetValue(itemData, action);
    a->setData(itemData);

    connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(slotRemoveSelectedAction(QAction*)));
    menu.exec(event->globalPos());

    return true;
}

void QDesignerMenuBar::slotRemoveSelectedAction(QAction *action)
{
    QAction *a = qvariant_cast<QAction*>(action->data());
    Q_ASSERT(a != 0);
    removeAction(a);
}

bool QDesignerMenuBar::eventFilter(QObject *object, QEvent *event)
{
    if (object == qApp->activePopupWidget())
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::ContextMenu:
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            QWidget *widget = qobject_cast<QWidget*>(object);

            if (widget && (widget == this || isAncestorOf(widget)))
                return handleEvent(widget, event);
        } break;
    }

    return false;
};

int QDesignerMenuBar::findAction(const QPoint &pos) const
{
    QList<QAction*> lst = actions();
    int index = 0;
    for (; index<lst.size() - 1; ++index) {
        QRect g = actionGeometry(lst.at(index));
        g.setTopLeft(QPoint(0, 0));

        if (g.contains(pos))
            break;
    }

    return index;
}

void QDesignerMenuBar::adjustIndicator(const QPoint &pos)
{
    if (QAction *action = actions().at(findAction(pos))) {
        if (action->menu()) {
            setActiveAction(action);
            action->menu()->setActiveAction(0);
        }

        if (QDesignerActionProviderExtension *a = actionProvider()) {
            a->adjustIndicator(pos);
        }
    }
}

void QDesignerMenuBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    }
}

void QDesignerMenuBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    }
}

void QDesignerMenuBar::dragLeaveEvent(QDragLeaveEvent *)
{
    adjustIndicator(QPoint(-1, -1));
}

void QDesignerMenuBar::dropEvent(QDropEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        event->acceptProposedAction();

        QAction *action = d->items.first();
        if (action && action->menu() && !actions().contains(action)) {
            int index = findAction(event->pos());
            index = qMin(index, actions().count() - 1);
            insertAction(actions().at(index), action);
        }
    }

    adjustIndicator(QPoint(-1, -1));
}

void QDesignerMenuBar::actionEvent(QActionEvent *event)
{
    QMenuBar::actionEvent(event);

    if (!m_blockSentinelChecker && event->type() == QEvent::ActionAdded
            && m_sentinel && event->action() != m_sentinel)
        m_sentinelChecker->start(0);
}

QDesignerFormWindowInterface *QDesignerMenuBar::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerMenuBar*>(this));
}

void QDesignerMenuBar::slotCheckSentinel()
{
    bool blocked = blockSentinelChecker(true);
    m_sentinelChecker->stop();
    removeAction(m_sentinel);
    addAction(m_sentinel);
    blockSentinelChecker(blocked);
}

bool QDesignerMenuBar::blockSentinelChecker(bool b)
{
    bool old = m_blockSentinelChecker;
    m_blockSentinelChecker = b;
    return old;
}

QDesignerActionProviderExtension *QDesignerMenuBar::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}
