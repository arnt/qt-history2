#include "qaccessiblewidgets.h"

#include <qstyle.h>
#include <qgroupbox.h>
#include <qscrollview.h>
#include <qlistbox.h>
#include <qwidgetstack.h>

/*!
  \class QAccessibleViewport qaccessiblewidget.h
  \brief The QAccessibleViewport class hides the viewport of scrollviews for accessibility.
  \internal
*/

QAccessibleViewport::QAccessibleViewport(QWidget *o, QWidget *sv)
    : QAccessibleWidget(o)
{
    Q_ASSERT(sv->inherits("QScrollView"));
    scrollview = (QScrollView*)sv;
}

QAccessibleScrollView *QAccessibleViewport::scrollView() const
{
    QAccessibleInterface *iface = 0;
    queryAccessibleInterface(scrollview, &iface);
    Q_ASSERT(iface);
    return (QAccessibleScrollView *)iface;
}

int QAccessibleViewport::childAt(int x, int y) const
{
    int child = QAccessibleWidget::childAt(x, y);
    if (child > 0)
        return child;

    QPoint p = widget()->mapFromGlobal(QPoint(x,y));
    return scrollView()->itemAt(p.x(), p.y());
}

QRect QAccessibleViewport::rect(int child) const
{
    if (!child)
        return QAccessibleWidget::rect(child);
    QRect rect = scrollView()->itemRect(child);
    QPoint tl = widget()->mapToGlobal(QPoint(0,0));
    return QRect(tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height());
}

/*
int QAccessibleViewport::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    // ### call itemUp/Down etc. here
    const int items = scrollView()->itemCount();
    switch(direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return items;
    case NavNext:
    case NavDown:
        return startControl + 1 > items ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
        return startControl - 1 < 1 ? -1 : startControl - 1;
    default:
        break;
    }

    return -1;
}
*/

int QAccessibleViewport::childCount() const
{
    int widgets = QAccessibleWidget::childCount();
    return widgets ? widgets : scrollView()->itemCount();
}

QString QAccessibleViewport::text(Text t, int child) const
{
    return scrollView()->text(t, child);
}

bool QAccessibleViewport::doAction(int action, int child)
{
    return scrollView()->doAction(action, child);
}

QAccessible::Role QAccessibleViewport::role(int child) const
{
    return scrollView()->role(child);
}

int QAccessibleViewport::state(int child) const
{
    return scrollView()->state(child);
}

bool QAccessibleViewport::setSelected(int child, bool on, bool extend)
{
//###    return scrollView()->setSelected(child, on, extend);
    return 0;
}

void QAccessibleViewport::clearSelection()
{
//###    scrollView()->clearSelection();
}

QVector<int> QAccessibleViewport::selection() const
{
//###    return scrollView()->selection();
    return QVector<int>();
}

/*!
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
*/

/*!
  Constructs a QAccessibleScrollView object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollView::QAccessibleScrollView(QWidget *w, Role role)
: QAccessibleWidget(w, role)
{
}

/*!
  Returns the ID of the item at viewport position \a x, \a y.
*/
int QAccessibleScrollView::itemAt(int /*x*/, int /*y*/) const
{
    return 0;
}

/*!
  Returns the location of the item with ID \a item in viewport coordinates.
*/
QRect QAccessibleScrollView::itemRect(int /*item*/) const
{
    return QRect();
}

/*!
  Returns the number of items.
*/
int QAccessibleScrollView::itemCount() const
{
    return 0;
}

/*!
  \class QAccessibleListBox qaccessiblewidget.h
  \brief The QAccessibleListBox class implements the QAccessibleInterface for list boxes.
*/

/*!
  Constructs a QAccessibleListBox object for \a o.
*/
QAccessibleListBox::QAccessibleListBox(QWidget *o)
    : QAccessibleScrollView(o, List)
{
    Q_ASSERT(widget()->inherits("QListBox"));
}

/*! Returns the list box. */
QListBox *QAccessibleListBox::listBox() const
{
    return (QListBox*)widget();
}

/*! \reimp */
int QAccessibleListBox::itemAt(int x, int y) const
{
    QListBoxItem *item = listBox()->itemAt(QPoint(x, y));
    return listBox()->index(item) + 1;
}

/*! \reimp */
QRect QAccessibleListBox::itemRect(int item) const
{
    return listBox()->itemRect(listBox()->item(item-1));
}

/*! \reimp */
int QAccessibleListBox::itemCount() const
{
    return listBox()->count();
}

/*! \reimp */
QString QAccessibleListBox::text(Text t, int child) const
{
    if (!child || t != Name)
        return QAccessibleScrollView::text(t, child);

    QListBoxItem *item = listBox()->item(child - 1);
    if (item)
        return item->text();
    return QString();
}

/*! \reimp */
QAccessible::Role QAccessibleListBox::role(int child) const
{
    if (!child)
        return QAccessibleScrollView::role(child);
    return ListItem;
}

/*! \reimp */
int QAccessibleListBox::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QListBoxItem *item;
    if (!child || !(item = listBox()->item(child - 1)))
        return state;

    if (item->isSelectable()) {
        if (listBox()->selectionMode() == QListBox::Multi)
            state |= MultiSelectable;
        else if (listBox()->selectionMode() == QListBox::Extended)
            state |= ExtSelectable;
        else if (listBox()->selectionMode() == QListBox::Single)
            state |= Selectable;
        if (item->isSelected())
            state |= Selected;
    }
    if (listBox()->focusPolicy() != QWidget::NoFocus) {
        state |= Focusable;
        if (item->isCurrent())
            state |= Focused;
    }
    if (!listBox()->itemVisible(item))
        state |= Invisible;

    return state;
}

/*! \reimp
bool QAccessibleListBox::setFocus(int child)
{
    bool res = QAccessibleScrollView::setFocus(0);
    if (!child || !res)
        return res;

    QListBoxItem *item = listBox()->item(child -1);
    if (!item)
        return false;
    listBox()->setCurrentItem(item);
    return true;
}*/

/*! \reimp */
bool QAccessibleListBox::setSelected(int child, bool on, bool extend)
{
    if (!child || (extend &&
        listBox()->selectionMode() != QListBox::Extended &&
        listBox()->selectionMode() != QListBox::Multi))
        return false;

    QListBoxItem *item = listBox()->item(child -1);
    if (!item)
        return false;
    if (!extend) {
        listBox()->setSelected(item, on);
    } else {
        int current = listBox()->currentItem();
        bool down = child > current;
        for (int i = current; i != child;) {
            down ? i++ : i--;
            listBox()->setSelected(i, on);
        }

    }
    return true;
}

/*! \reimp */
void QAccessibleListBox::clearSelection()
{
    listBox()->clearSelection();
}

/*! \reimp */
QVector<int> QAccessibleListBox::selection() const
{
    QVector<int> array;
    uint size = 0;
    const uint c = listBox()->count();
    array.resize(c);
    for (uint i = 0; i < c; ++i) {
        if (listBox()->isSelected(i)) {
            ++size;
            array[(int)size-1] = i+1;
        }
    }
    array.resize(size);
    return array;
}
