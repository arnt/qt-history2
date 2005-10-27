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
#include "qdesigner_menubar_p.h"
#include "qdesigner_toolbar_p.h"
#include "actionrepository_p.h"
#include "actionprovider_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QRubberBand>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

using namespace qdesigner_internal;

QDesignerMenu::QDesignerMenu(QWidget *parent)
    : QMenu(parent)
{
    m_interactive = true;
    m_currentIndex = 0;

    setContextMenuPolicy(Qt::DefaultContextMenu);
    setAcceptDrops(true); // ### fake

    m_addItem = new SpecialMenuAction(this);
    m_addItem->setText(tr("new action"));
    addAction(m_addItem);

    m_addSeparator = new SpecialMenuAction(this);
    m_addSeparator->setText(tr("new separator"));
    addAction(m_addSeparator);

    m_showSubMenuTimer = new QTimer(this);
    connect(m_showSubMenuTimer, SIGNAL(timeout()), this, SLOT(slotShowSubMenuNow()));

    m_editor = new QLineEdit(this);
    m_editor->setObjectName("__qt__passive_editor");
    m_editor->hide();
    m_editor->installEventFilter(this);

    m_editor->installEventFilter(this);
    qApp->installEventFilter(this);
}

QDesignerMenu::~QDesignerMenu()
{
}

bool QDesignerMenu::handleEvent(QWidget *widget, QEvent *event)
{
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
        case QEvent::KeyPress:
            return handleKeyPressEvent(widget, static_cast<QKeyEvent*>(event));
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

bool QDesignerMenu::handleKeyPressEvent(QWidget *widget, QKeyEvent *e)
{
    m_showSubMenuTimer->stop();

    if (m_editor->isHidden()) { // In navigation mode
        switch (e->key()) {

        case Qt::Key_Delete:
#if 0 // ### implement me
            hideMenu();
            deleteMenu();
#endif
            break;

        case Qt::Key_Left:
            e->accept();
            moveLeft();
            return true;

        case Qt::Key_Right:
            e->accept();
            moveRight();
            return true; // no update

        case Qt::Key_Up:
            e->accept();
            moveUp(e->modifiers() & Qt::ControlModifier);
            return true;

        case Qt::Key_Down:
            e->accept();
            moveDown(e->modifiers() & Qt::ControlModifier);
            return true;

        case Qt::Key_PageUp:
            m_currentIndex = 0;
            break;

        case Qt::Key_PageDown:
            m_currentIndex = actions().count() - 1;
            break;

        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_F2:
            e->accept();
            enterEditMode();
            return true; // no update

        case Qt::Key_Escape:
            e->ignore();
            setFocus();
            hide();
            closeMenuChain();
            return true;

        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Control:
            e->ignore();
            setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
            return true; // no update

        default:
            if (!e->text().isEmpty() && e->text().at(0).toLatin1() >= 32) {
                showLineEdit();
                QApplication::sendEvent(m_editor, e);
                e->accept();
            } else {
                e->ignore();
            }
            return true;
        }
    } else { // In edit mode
        switch (e->key()) {
        default:
            return false;

        case Qt::Key_Control:
            e->ignore();
            return true;

        case Qt::Key_Enter:
        case Qt::Key_Return:
            leaveEditMode();
            m_editor->hide();
            setFocus();
            break;

        case Qt::Key_Escape:
            m_editor->hide();
            setFocus();
            break;
        }
    }

    e->accept();
    update();

    return true;

#if 0
    event->accept();

    switch (event->key()) {
        case Qt::Key_Escape:
            hide();
            break;
        case Qt::Key_Up:
            moveUp();
            break;
        case Qt::Key_Down:
            moveDown();
            break;
        case Qt::Key_Left:
            moveLeft();
            break;
        case Qt::Key_Right:
            moveRight();
            break;

        default:
            break;
    }

    return true;
#endif
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

    return true;
}

bool QDesignerMenu::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
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

void QDesignerMenu::paintEvent(QPaintEvent *event)
{
    QMenu::paintEvent(event);

    if (QAction *a = currentAction()) {
        QPainter p(this);
        QRect g = actionGeometry(a);
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(g.adjusted(0, 0, -1, -1));
    }
}

bool QDesignerMenu::eventFilter(QObject *object, QEvent *event)
{
    if (object != this && object != m_editor)
        return false;

    if (object == m_editor && event->type() == QEvent::FocusOut) {
        leaveEditMode();
        m_editor->hide();
        update();
        return false;
    }

    switch (event->type()) {
        default: break;

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
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
}

QDesignerFormWindowInterface *QDesignerMenu::formWindow() const
{
    if (parentMenu())
        return parentMenu()->formWindow();

    return QDesignerFormWindowInterface::findFormWindow(parentWidget());
}

QDesignerActionProviderExtension *QDesignerMenu::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}

void QDesignerMenu::closeMenuChain()
{
    m_showSubMenuTimer->stop();

    QWidget *w = this;
    while (w && qobject_cast<QMenu*>(w))
        w = w->parentWidget();

    if (w) {
        foreach (QMenu *subMenu, qFindChildren<QMenu*>(w)) {
            subMenu->hide();
        }
    }
}

void QDesignerMenu::moveLeft()
{
    if (parentMenu()) {
        hide();
    } else if (QDesignerMenuBar *mb = parentMenuBar()) {
        hide();
        mb->moveLeft();
    }
    updateCurrentAction();
}

void QDesignerMenu::moveRight()
{
    closeMenuChain();
    if (QDesignerMenuBar *mb = parentMenuBar()) {
        mb->moveRight();
    }
}

void QDesignerMenu::moveUp(bool ctrl)
{
    if (m_currentIndex == 0) {
        hide();
        return;
    }
    m_currentIndex = qMax(0, --m_currentIndex);
    updateCurrentAction();
}

void QDesignerMenu::moveDown(bool ctrl)
{
    if (m_currentIndex == actions().count() - 1) {
        hide();
        return;
    }
    m_currentIndex = qMin(actions().count() - 1, ++m_currentIndex);
    updateCurrentAction();
}

QAction *QDesignerMenu::currentAction() const
{
    if (m_currentIndex < 0 || m_currentIndex >= actions().count())
        return 0;

    return actions().at(m_currentIndex);
}

int QDesignerMenu::realActionCount()
{
    return actions().count() - 2; // 2 fake actions
}

void QDesignerMenu::updateCurrentAction()
{
    update();

    showSubMenu(currentAction());
}

void QDesignerMenu::createRealMenuAction(QAction *action) // ### undo/redo
{
    if (action->menu())
        return; // nothing to do

    QDesignerMenu *tempMenu = findOrCreateSubMenu(action);
    m_subMenus.remove(action);

    action->setMenu(tempMenu);

    Q_ASSERT(formWindow() != 0);
    QDesignerFormWindowInterface *fw = formWindow();
    fw->core()->metaDataBase()->add(action->menu());
}

QDesignerMenu *QDesignerMenu::findOrCreateSubMenu(QAction *action)
{
    if (action->menu())
        return qobject_cast<QDesignerMenu*>(action->menu());

    QDesignerMenu *menu = m_subMenus.value(action);
    if (!menu) {
        menu = new QDesignerMenu(this);
        m_subMenus.insert(action, menu);
    }

    return menu;
}

void QDesignerMenu::slotShowSubMenuNow()
{
    m_showSubMenuTimer->stop();
    QAction *action = currentAction();

    if (QMenu *menu = findOrCreateSubMenu(action)) {
        menu->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        menu->adjustSize();
        QRect g = actionGeometry(action);
        menu->move(mapToGlobal(g.topRight()));
        menu->show();
    }
}

void QDesignerMenu::showSubMenu(QAction *action)
{
    m_showSubMenuTimer->stop();

    if (!action || qobject_cast<SpecialMenuAction*>(action)
            || action->isSeparator() || !isVisible())
        return;

    m_showSubMenuTimer->start(1000);
}

QDesignerMenu *QDesignerMenu::parentMenu() const
{
    return qobject_cast<QDesignerMenu*>(parentWidget());
}

QDesignerMenuBar *QDesignerMenu::parentMenuBar() const
{
    if (QDesignerMenuBar *mb = qobject_cast<QDesignerMenuBar*>(parentWidget())) {
        return mb;
    } else if (QDesignerMenu *m = parentMenu()) {
        return m->parentMenuBar();
    }

    return 0;
}

void QDesignerMenu::setVisible(bool visible)
{
    if (visible)
        m_currentIndex = 0;

    QMenu::setVisible(visible);

}

void QDesignerMenu::adjustSpecialActions()
{
    removeAction(m_addItem);
    removeAction(m_addSeparator);
    addAction(m_addItem);
    addAction(m_addSeparator);
}

bool QDesignerMenu::interactive(bool i)
{
    bool old = m_interactive;
    m_interactive = i;
    return old;
}

void QDesignerMenu::enterEditMode()
{
    if (m_currentIndex < realActionCount()) {
        showLineEdit();
    } else {
        QAction *sep = createAction();
        sep->setSeparator(true);
        insertAction(currentAction(), sep);
    }
}

void QDesignerMenu::leaveEditMode()
{
    QAction *action = 0;

    if (m_currentIndex < realActionCount()) {
        action = actions().at(m_currentIndex);
    } else {
        Q_ASSERT(formWindow() != 0);   // ### undo/redo
        action = createAction();
        insertAction(currentAction(), action);
    }

    action->setText(m_editor->text()); // ### undo/redo
    adjustSize();

    if (parentMenu()) { // ### undo/redo
        parentMenu()->createRealMenuAction(parentMenu()->currentAction());
    }
}

QAction *QDesignerMenu::safeMenuAction(QDesignerMenu *menu) const
{
    QAction *action = menu->menuAction();

    if (!action)
        action = m_subMenus.key(menu);

    return action;
}

void QDesignerMenu::showLineEdit()
{
    QAction *action = 0;

    if (m_currentIndex < realActionCount())
        action = actions().at(m_currentIndex);
    else
        action = m_addItem;

    if (action->isSeparator())
        return;

    // open edit field for item name
    m_editor->setText(action->text());
    m_editor->selectAll();
    m_editor->setGeometry(actionGeometry(action));
    m_editor->show();
    m_editor->setFocus();
}

QAction *QDesignerMenu::createAction() // ### undo/redo
{
    Q_ASSERT(formWindow() != 0);
    QDesignerFormWindowInterface *fw = formWindow();

    QAction *action = new QAction(fw);
    action->setObjectName("action");
    fw->core()->metaDataBase()->add(action);
    fw->ensureUniqueObjectName(action);
    return action;
}

