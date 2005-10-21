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

#include "qdesigner_toolbar_p.h"
// #include "actionrepository_p.h"
#include "actionprovider_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
// Q_DECLARE_METATYPE(QListWidgetItem*)

using namespace qdesigner_internal;

QDesignerToolBar::QDesignerToolBar(QWidget *parent)
    : QToolBar(parent)
{
    m_blockSentinelChecker = false;
    m_sentinel = 0;

    setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true); // ### fake

    Sentinel *btn = new Sentinel(this);
    connect(btn, SIGNAL(clicked()), this, SLOT(slotNewToolBar()));

    m_sentinel = addWidget(btn);
    addAction(m_sentinel);

    m_sentinelChecker = new QTimer(this);
    connect(m_sentinelChecker, SIGNAL(timeout()), this, SLOT(slotCheckSentinel()));

    qApp->installEventFilter(this);
}

QDesignerToolBar::~QDesignerToolBar()
{
}

bool QDesignerToolBar::handleEvent(QWidget *widget, QEvent *event)
{
    if (!formWindow() || isPassiveWidget(widget))
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

void QDesignerToolBar::startDrag(const QPoint &pos)
{
    int index = findAction(pos);
/*    if (index == actions().count() - 1)
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
    } */
}

bool QDesignerToolBar::handleMousePressEvent(QWidget *, QMouseEvent *event)
{
    event->accept();
    m_startPosition = QPoint();

    if (event->button() != Qt::LeftButton)
        return true;

    if (QDesignerFormWindowInterface *fw = formWindow()) {
        if (QDesignerPropertyEditorInterface *pe = fw->core()->propertyEditor()) {
            pe->setObject(this);
        }
    }

    m_startPosition = mapFromGlobal(event->globalPos());

    return true;
}

bool QDesignerToolBar::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    m_startPosition = QPoint();

    return true;
}

bool QDesignerToolBar::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    if (m_startPosition.isNull())
        return true;

    QPoint pos = mapFromGlobal(event->globalPos());

    if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance())
        return true;

    startDrag(pos);
    m_startPosition = QPoint();

    return true;
}

bool QDesignerToolBar::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
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

void QDesignerToolBar::slotRemoveSelectedAction(QAction *action)
{
    QAction *a = qvariant_cast<QAction*>(action->data());
    Q_ASSERT(a != 0);
    removeAction(a);
}

bool QDesignerToolBar::eventFilter(QObject *object, QEvent *event)
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

int QDesignerToolBar::findAction(const QPoint &pos) const
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

QDesignerActionProviderExtension *QDesignerToolBar::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}

void QDesignerToolBar::adjustIndicator(const QPoint &pos)
{
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(pos);
    }
}

void QDesignerToolBar::dragEnterEvent(QDragEnterEvent *event)
{
/*    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    } */
}

void QDesignerToolBar::dragMoveEvent(QDragMoveEvent *event)
{
/*    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    } */
}

void QDesignerToolBar::dragLeaveEvent(QDragLeaveEvent *)
{
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(QPoint(-1,-1));
    }
}

void QDesignerToolBar::dropEvent(QDropEvent *event)
{
/*    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        event->acceptProposedAction();

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            int index = findAction(event->pos());
            index = qMin(index, actions().count() - 1);
            insertAction(actions().at(index), action);
        }
    }

    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(QPoint(-1,-1));
    } */
}

void QDesignerToolBar::actionEvent(QActionEvent *event)
{
    QToolBar::actionEvent(event);

    if (!m_blockSentinelChecker && event->type() == QEvent::ActionAdded
            && m_sentinel && event->action() != m_sentinel)
        m_sentinelChecker->start(0);
}

QDesignerFormWindowInterface *QDesignerToolBar::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerToolBar*>(this));
}

bool QDesignerToolBar::isPassiveWidget(QWidget *widget) const
{
    if (qobject_cast<Sentinel*>(widget))
        return true;
    else if (!qstrcmp(widget->metaObject()->className(), "QToolBarHandle"))
        return true;

    return false;
}

void QDesignerToolBar::slotCheckSentinel()
{
    bool blocked = blockSentinelChecker(true);
    m_sentinelChecker->stop();
    removeAction(m_sentinel);
    addAction(m_sentinel);
    blockSentinelChecker(blocked);
}

bool QDesignerToolBar::blockSentinelChecker(bool b)
{
    bool old = m_blockSentinelChecker;
    m_blockSentinelChecker = b;
    return old;
}

void QDesignerToolBar::slotNewToolBar()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        QToolBar *tb = qobject_cast<QToolBar*>(core->widgetFactory()->createWidget("QToolBar", fw->mainContainer()));
        core->metaDataBase()->add(tb);

        if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), fw->mainContainer())) {
            container->addWidget(tb);
        }
    }
}

SentinelAction::SentinelAction(QWidget *widget)
    : QAction(widget)
{
    setSeparator(true);
}

SentinelAction::~SentinelAction()
{
}

Sentinel::Sentinel(QWidget *widget)
    : QToolButton(widget)
{
    setObjectName(QString::fromUtf8("__qt__passive_new"));
    setText(tr("=>")); // ### replace with something else
    setToolButtonStyle(Qt::ToolButtonTextOnly);
}

Sentinel::~Sentinel()
{
}

