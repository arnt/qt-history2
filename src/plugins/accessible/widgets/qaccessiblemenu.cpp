#include "qaccessiblemenu.h"

#include <qaccel.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstyle.h>

QString Q_GUI_EXPORT qacc_stripAmp(const QString &text);
QString Q_GUI_EXPORT qacc_hotKey(const QString &text);

class MyPopupMenu : public QPopupMenu
{
    friend class QAccessiblePopup;
};

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
    return popupMenu()->count();
}

QRect QAccessiblePopup::rect(int child) const
{
    if (!child)
	return QAccessibleWidget::rect(child);

    QRect r = popupMenu()->itemGeometry(child - 1);
    QPoint tlp = popupMenu()->mapToGlobal(QPoint(0,0));

    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessiblePopup::childAt(int x, int y) const
{
    QPoint p = popupMenu()->mapFromGlobal(QPoint(x,y));
    MyPopupMenu *mp = (MyPopupMenu*)popupMenu();
    return mp->itemAtPos(p, FALSE) + 1;
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

    int id;
    if (child)
	id = popupMenu()->idAt(child - 1);

    switch (t) {
    case Name:
	if (!child)
	    return popupMenu()->windowTitle();
	return qacc_stripAmp(popupMenu()->text(id));
    case Help:
	return popupMenu()->whatsThis(id);
    case Accelerator:
	return (QString)popupMenu()->accel(id);
    default:
	break;
    }
    return tx;
}

QAccessible::Role QAccessiblePopup::role(int child) const
{
    if (!child)
	return PopupMenu;

    QMenuItem *item = popupMenu()->findItem(popupMenu()->idAt(child -1));
    if (item && item->isSeparator())
	return Separator;
    return MenuItem;
}

int QAccessiblePopup::state(int child) const
{
    int s = QAccessibleWidget::state(child);
    if (!child)
	return s;

    int id = popupMenu()->idAt(child -1);
    QMenuItem *item = popupMenu()->findItem(id);
    if (!item)
	return s;

    if (popupMenu()->style().styleHint(QStyle::SH_Menu_MouseTracking))
	s |= HotTracked;
    if (item->isSeparator() || !item->isEnabled())
	s |= Unavailable;
    if (popupMenu()->isCheckable() && item->isChecked())
	s |= Checked;
    if (popupMenu()->isItemActive(id))
	s |= Focused;

    return s;
}

bool QAccessiblePopup::doAction(int action, int child)
{
    if (!child)
	return FALSE;

    int id = popupMenu()->idAt(child -1);
    QMenuItem *item = popupMenu()->findItem(id);
    if (!item || !item->isEnabled())
	return FALSE;

    popupMenu()->activateItemAt(child - 1);
    return TRUE;
}

/*bool QAccessiblePopup::setFocus(int child)
{
    if (!child)
	return FALSE;

    int id = popupMenu()->idAt(child -1);
    QMenuItem *item = popupMenu()->findItem(id);
    if (!item || !item->isEnabled())
	return FALSE;

    popupMenu()->setActiveItem(child - 1);
    return TRUE;
}
*/

class MyMenuBar : public QMenuBar
{
    friend class QAccessibleMenuBar;
};

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
    return menuBar()->count();
}

QRect QAccessibleMenuBar::rect(int child) const
{
    if (!child)
	return QAccessibleWidget::rect(child);

    MyMenuBar *mb = (MyMenuBar*)menuBar();
    QRect r = mb->itemRect(child - 1);
    QPoint tlp = mb->mapToGlobal(QPoint(0,0));

    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessibleMenuBar::childAt(int x, int y) const
{
    MyMenuBar *mb = (MyMenuBar*)menuBar();
    QPoint p = mb->mapFromGlobal(QPoint(x,y));
    return mb->itemAtPos(p) + 1;
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

    int id = child ? menuBar()->idAt(child - 1) : -1;

    if (id != -1) switch (t) {
    case Name:
	return qacc_stripAmp(menuBar()->text(id));
    case Accelerator:
	str = qacc_hotKey(menuBar()->text(id));
	break;
    default:
	break;
    }

    if (str.isEmpty())
	str = QAccessibleWidget::text(t, child);
    return str;
}

QAccessible::Role QAccessibleMenuBar::role(int child) const
{
    if (!child)
	return MenuBar;

    QMenuItem *item = menuBar()->findItem(menuBar()->idAt(child -1));
    if (item && item->isSeparator())
	return Separator;
    return MenuItem;
}

int QAccessibleMenuBar::state(int child) const
{
    int s = QAccessibleWidget::state(child);
    if (!child)
	return s;

    int id = menuBar()->idAt(child -1);
    QMenuItem *item = menuBar()->findItem(id);
    if (!item)
	return s;

    if (menuBar()->style().styleHint(QStyle::SH_Menu_MouseTracking))
	s |= HotTracked;
    if (item->isSeparator() || !item->isEnabled())
	s |= Unavailable;
    if (menuBar()->isItemActive(id))
	s |= Focused;

    return s;
}

bool QAccessibleMenuBar::doAction(int action, int child)
{
    if (!child)
	return FALSE;

    int id = menuBar()->idAt(child -1);
    QMenuItem *item = menuBar()->findItem(id);
    if (!item || !item->isEnabled())
	return FALSE;

    menuBar()->activateItemAt(child - 1);
    return TRUE;
}

/*
bool QAccessibleMenuBar::setFocus(int child)
{
    if (!child)
	return FALSE;

    int id = menuBar()->idAt(child -1);
    QMenuItem *item = menuBar()->findItem(id);
    if (!item || !item->isEnabled())
	return FALSE;

    return TRUE;
}
*/
