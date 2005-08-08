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

#include "qdesigner_menu_p.h"
#include "qdesigner_toolbar_p.h"
#include "actionrepository_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QRubberBand>
#include <QtGui/QMenu>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

QDesignerMenu::QDesignerMenu(QWidget *parent)
    : QMenu(parent)
{
    m_blockSentinelChecker = false;
    m_sentinel = 0;

    setContextMenuPolicy(Qt::DefaultContextMenu);
    m_indicator = new QRubberBand(QRubberBand::Line, this);
    m_indicator->hide();

    setAcceptDrops(true); // ### fake

    m_sentinel = new SentinelAction(this);       // ### use a special widget as indicator
    addAction(m_sentinel);

    m_sentinelChecker = new QTimer(this);
    connect(m_sentinelChecker, SIGNAL(timeout()), this, SLOT(slotCheckSentinel()));

    qApp->installEventFilter(this);
}

QDesignerMenu::~QDesignerMenu()
{
}

bool QDesignerMenu::handleEvent(QWidget *widget, QEvent *event)
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

bool QDesignerMenu::handleMousePressEvent(QWidget *, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->accept();
        return true;
    }

    QPoint pos = mapFromGlobal(event->globalPos());
    int index = findAction(pos);
    if (index >= actions().count() - 1)
        return false;

    QAction *action = actions().at(index);
    removeAction(action);

    adjustSize();
    adjustIndicator(event->pos());

    QDrag *drag = new QDrag(this);
    drag->setPixmap(action->icon().pixmap(QSize(22, 22)));

    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    data->items.append(action);
    drag->setMimeData(data);

    if (drag->start() == Qt::IgnoreAction) {
        QAction *previous = actions().at(index);
        insertAction(previous, action);
    }

    return true;
}

bool QDesignerMenu::handleMouseReleaseEvent(QWidget *, QMouseEvent *)
{
    return true;
}

bool QDesignerMenu::handleMouseMoveEvent(QWidget *, QMouseEvent *)
{
    return true;
}

bool QDesignerMenu::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
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

void QDesignerMenu::slotRemoveSelectedAction(QAction *action)
{
    QAction *a = qvariant_cast<QAction*>(action->data());
    Q_ASSERT(a != 0);
    removeAction(a);
    adjustSize();
}

bool QDesignerMenu::eventFilter(QObject *object, QEvent *event)
{
    if (object == qApp->activePopupWidget() && !qobject_cast<QDesignerMenu*>(object))
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

int QDesignerMenu::findAction(const QPoint &pos) const
{
    for (int i = 0; i<actions().size() - 1; ++i) {
        QRect g = actionGeometry(actions().at(i));
        g.setTopLeft(QPoint(0, 0));

        if (g.contains(pos)) {
            if (pos.x() > g.right() - 10) // ### 10px
                return i + 1;

            return i;
        }
    }

    return actions().size() - 1; // the sentinel
}

void QDesignerMenu::adjustIndicator(const QPoint &pos)
{
    QRect g = actionGeometry(actions().at(findAction(pos)));
    g.moveLeft(0);
    g.setWidth(width());
    g.setHeight(2);

    m_indicator->setGeometry(g);
}

void QDesignerMenu::dragEnterEvent(QDragEnterEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
            m_indicator->show();
        }
    }
}

void QDesignerMenu::dragMoveEvent(QDragMoveEvent *event)
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

void QDesignerMenu::dragLeaveEvent(QDragLeaveEvent *)
{
    m_indicator->hide();
}

void QDesignerMenu::dropEvent(QDropEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        event->acceptProposedAction();

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            int index = findAction(event->pos());
            index = qMin(index, actions().count() - 1);
            insertAction(actions().at(index), action);
            adjustSize();
        }
    }

    m_indicator->hide();
}

void QDesignerMenu::actionEvent(QActionEvent *event)
{
    QMenu::actionEvent(event);

    if (!m_blockSentinelChecker && event->type() == QEvent::ActionAdded
            && m_sentinel && event->action() != m_sentinel)
        m_sentinelChecker->start(0);
}

QDesignerFormWindowInterface *QDesignerMenu::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(parentWidget());
}

void QDesignerMenu::slotCheckSentinel()
{
    bool blocked = blockSentinelChecker(true);
    m_sentinelChecker->stop();
    removeAction(m_sentinel);
    addAction(m_sentinel);
    blockSentinelChecker(blocked);
}

bool QDesignerMenu::blockSentinelChecker(bool b)
{
    bool old = m_blockSentinelChecker;
    m_blockSentinelChecker = b;
    return old;
}
