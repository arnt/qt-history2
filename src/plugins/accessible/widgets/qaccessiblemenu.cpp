#include "qaccessiblemenu.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstyle.h>

QString Q_GUI_EXPORT qacc_stripAmp(const QString &text);
QString Q_GUI_EXPORT qacc_hotKey(const QString &text);

QAccessiblePopup::QAccessiblePopup(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(popupMenu());
}

QPopupMenu *QAccessiblePopup::popupMenu() const
{
    return qt_cast<QPopupMenu*>(object());
}

int QAccessiblePopup::childCount() const
{
    return popupMenu()->actions().count();
}

QRect QAccessiblePopup::rect(int child) const
{
    if (!child || child >= childCount())
        return QAccessibleWidget::rect(child);

    QRect r = popupMenu()->actionGeometry(popupMenu()->actions()[child - 1]);
    QPoint tlp = popupMenu()->mapToGlobal(QPoint(0,0));

    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessiblePopup::childAt(int x, int y) const
{
    QPoint p = popupMenu()->mapFromGlobal(QPoint(x,y));
    return popupMenu()->actions().indexOf(popupMenu()->actionAtPos(p, false)) + 1;
}

/*int QAccessiblePopup::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    switch (direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return childCount();
    case NavNext:
    case NavDown:
        return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
        return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
        break;
    }
    return -1;
}*/

QString QAccessiblePopup::text(Text t, int child) const
{
    QString tx = QAccessibleWidget::text(t, child);
    if (tx.size())
        return tx;

    switch (t) {
    case Name:
        if (!child)
            return popupMenu()->windowTitle();
        return qacc_stripAmp(popupMenu()->actions()[child-1]->text());
    case Help:
        return popupMenu()->actions()[child-1]->whatsThis();
    case Accelerator:
        return (QString)popupMenu()->actions()[child-1]->accel();
    default:
        break;
    }
    return tx;
}

QAccessible::Role QAccessiblePopup::role(int child) const
{
    if (!child)
        return PopupMenu;

    QAction *action = popupMenu()->actions()[child-1];
    if (action && action->isSeparator())
        return Separator;
    return MenuItem;
}

int QAccessiblePopup::state(int child) const
{
    int s = QAccessibleWidget::state(child);
    if (!child)
        return s;

    QAction *action = popupMenu()->actions()[child-1];
    if (!action)
        return s;

    if (popupMenu()->style().styleHint(QStyle::SH_Menu_MouseTracking))
        s |= HotTracked;
    if (action->isSeparator() || !action->isEnabled())
        s |= Unavailable;
    if (popupMenu()->isCheckable() && action->isChecked())
        s |= Checked;
    if (popupMenu()->activeAction() == action)
        s |= Focused;

    return s;
}

bool QAccessiblePopup::doAction(int act, int child, const QVariantList &)
{
    if (!child)
        return false;

    QAction *action = popupMenu()->actions()[child-1];
    if (!action || !action->isEnabled())
        return false;
    action->activate(QAction::Trigger);
    return true;
}

/*bool QAccessiblePopup::setFocus(int child)
{
    if (!child)
        return false;

    int id = popupMenu()->idAt(child -1);
    QMenuItem *item = popupMenu()->findItem(id);
    if (!item || !item->isEnabled())
        return false;

    popupMenu()->setActiveItem(child - 1);
    return true;
}
*/

QAccessibleMenuBar::QAccessibleMenuBar(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(menuBar());
}

QMenuBar *QAccessibleMenuBar::menuBar() const
{
    return qt_cast<QMenuBar*>(object());
}

int QAccessibleMenuBar::childCount() const
{
    return menuBar()->actions().count();
}

QRect QAccessibleMenuBar::rect(int child) const
{
    if (!child)
        return QAccessibleWidget::rect(child);

    QRect r = menuBar()->actionGeometry(menuBar()->actions()[child - 1]);
    QPoint tlp = menuBar()->mapToGlobal(QPoint(0,0));
    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessibleMenuBar::childAt(int x, int y) const
{
    QPoint p = menuBar()->mapFromGlobal(QPoint(x,y));
    return menuBar()->actions().indexOf(menuBar()->actionAtPos(p)) + 1;
}

/*
int QAccessibleMenuBar::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    switch (direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return childCount();
    case NavNext:
    case NavRight:
        return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
        return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
        break;
    }

    return -1;
}
*/

QString QAccessibleMenuBar::text(Text t, int child) const
{
    QString str;

    if(child) {
        if(QAction *action = menuBar()->actions()[child]) {
            switch (t) {
            case Name:
                return qacc_stripAmp(action->text());
            case Accelerator:
                str = qacc_hotKey(action->text());
                break;
            default:
                break;
            }
        }
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);
    return str;
}

QAccessible::Role QAccessibleMenuBar::role(int child) const
{
    if (!child)
        return MenuBar;

    QAction *action = menuBar()->actions()[child-1];
    if (action && action->isSeparator())
        return Separator;
    return MenuItem;
}

int QAccessibleMenuBar::state(int child) const
{
    int s = QAccessibleWidget::state(child);
    if (!child)
        return s;

    QAction *action = menuBar()->actions()[child-1];
    if (!action)
        return s;

    if (menuBar()->style().styleHint(QStyle::SH_Menu_MouseTracking))
        s |= HotTracked;
    if (action->isSeparator() || !action->isEnabled())
        s |= Unavailable;
    if (menuBar()->activeAction() == action)
        s |= Focused;

    return s;
}

bool QAccessibleMenuBar::doAction(int act, int child, const QVariantList &)
{
    if (!child)
        return false;

    QAction *action = menuBar()->actions()[child-1];
    if (!action || !action->isEnabled())
        return false;
    action->activate(QAction::Trigger);
    return true;
}

/*
bool QAccessibleMenuBar::setFocus(int child)
{
    if (!child)
        return false;

    int id = menuBar()->idAt(child -1);
    QMenuItem *item = menuBar()->findItem(id);
    if (!item || !item->isEnabled())
        return false;

    return true;
}
*/
