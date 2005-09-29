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
#include "actionprovider_p.h"

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

void QDesignerMenu::startDrag(const QPoint &pos)
{
    int index = findAction(pos);
    if (index >= actions().count() - 1)
        return;

    QAction *action = actions().at(index);
    removeAction(action);
    adjustSize();

    adjustIndicator(pos);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(action->icon().pixmap(QSize(22, 22)));

    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    data->items.append(action);
    drag->setMimeData(data);

    if (drag->start() == Qt::IgnoreAction) {
        QAction *previous = actions().at(index);
        insertAction(previous, action);
        adjustSize();
    }
}

bool QDesignerMenu::handleMousePressEvent(QWidget *, QMouseEvent *event)
{
    m_startPosition = QPoint();
    event->accept();

    if (event->button() != Qt::LeftButton)
        return true;

    m_startPosition = mapFromGlobal(event->globalPos());

    return true;
}

bool QDesignerMenu::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    m_startPosition = QPoint();

    return false;
}

bool QDesignerMenu::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    if (m_startPosition.isNull())
        return false;

    QPoint pos = mapFromGlobal(event->globalPos());

    if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance())
        return false;

    startDrag(pos);
    m_startPosition = QPoint();

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
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(pos);
    }
}

void QDesignerMenu::dragEnterEvent(QDragEnterEvent *event)
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
    adjustIndicator(QPoint(-1, -1));
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

    adjustIndicator(QPoint(-1, -1));
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

QDesignerActionProviderExtension *QDesignerMenu::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}
