#include "qaccessiblewidgets.h"

#include <qstyle.h>
#include <qgroupbox.h>
#include <qscrollview.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qtextedit.h>
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
	return FALSE;
    listBox()->setCurrentItem(item);
    return TRUE;
}*/

/*! \reimp */
bool QAccessibleListBox::setSelected(int child, bool on, bool extend)
{
    if (!child || (extend &&
	listBox()->selectionMode() != QListBox::Extended &&
	listBox()->selectionMode() != QListBox::Multi))
	return FALSE;

    QListBoxItem *item = listBox()->item(child -1);
    if (!item)
	return FALSE;
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
    return TRUE;
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
	    array[ (int)size-1 ] = i+1;
	}
    }
    array.resize(size);
    return array;
}

/*!
  \class QAccessibleListView qaccessiblewidget.h
  \brief The QAccessibleListView class implements the QAccessibleInterface for list views.
*/

static QListViewItem *findLVItem(QListView* listView, int child)
{
    int id = 1;
    QListViewItemIterator it(listView);
    QListViewItem *item = it.current();
    while (item && id < child) {
	++it;
	++id;
	item = it.current();
    }
    return item;
}

/*!
  Constructs a QAccessibleListView object for \a o.
*/
QAccessibleListView::QAccessibleListView(QWidget *o)
    : QAccessibleScrollView(o, Tree)
{
}

/*! Returns the list view. */
QListView *QAccessibleListView::listView() const
{
    Q_ASSERT(widget()->inherits("QListView"));
    return (QListView*)widget();
}

/*! \reimp */
int QAccessibleListView::itemAt(int x, int y) const
{
    QListViewItem *item = listView()->itemAt(QPoint(x, y));
    if (!item)
	return 0;

    QListViewItemIterator it(listView());
    int c = 1;
    while (it.current()) {
	if (it.current() == item)
	    return c;
	++c;
	++it;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleListView::itemRect(int child) const
{
    QListViewItem *item = findLVItem(listView(), child);
    if (!item)
	return QRect();
    return listView()->itemRect(item);
}

/*! \reimp */
int QAccessibleListView::itemCount() const
{
    QListViewItemIterator it(listView());
    int c = 0;
    while (it.current()) {
	++c;
	++it;
    }

    return c;
}

/*! \reimp */
QString QAccessibleListView::text(Text t, int child) const
{
    if (!child || t != Name)
	return QAccessibleScrollView::text(t, child);

    QListViewItem *item = findLVItem(listView(), child);
    if (!item)
	return QString();
    return item->text(0);
}

/*! \reimp */
QAccessible::Role QAccessibleListView::role(int child) const
{
    if (!child)
	return QAccessibleScrollView::role(child);
    return TreeItem;
}

/*! \reimp */
int QAccessibleListView::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QListViewItem *item;
    if (!child || !(item = findLVItem(listView(), child)))
	return state;

    if (item->isSelectable()) {
	if (listView()->selectionMode() == QListView::Multi)
	    state |= MultiSelectable;
	else if (listView()->selectionMode() == QListView::Extended)
	    state |= ExtSelectable;
	else if (listView()->selectionMode() == QListView::Single)
	    state |= Selectable;
	if (item->isSelected())
	    state |= Selected;
    }
    if (listView()->focusPolicy() != QWidget::NoFocus) {
	state |= Focusable;
	if (item == listView()->currentItem())
	    state |= Focused;
    }
    if (item->childCount()) {
	if (item->isOpen())
	    state |= Expanded;
	else
	    state |= Collapsed;
    }
    if (!listView()->itemRect(item).isValid())
	state |= Invisible;

    if (item->rtti() == QCheckListItem::RTTI) {
	if (((QCheckListItem*)item)->isOn())
	    state|=Checked;
    }
    return state;
}

/*! \reimp
QAccessibleInterface *QAccessibleListView::focusChild(int *child) const
{
    QListViewItem *item = listView()->currentItem();
    if (!item)
	return 0;

    QListViewItemIterator it(listView());
    int c = 1;
    while (it.current()) {
	if (it.current() == item) {
	    *child = c;
	    return (QAccessibleInterface*)this;
	}
	++c;
	++it;
    }
    return 0;
}
*/
/*! \reimp
bool QAccessibleListView::setFocus(int child)
{
    bool res = QAccessibleScrollView::setFocus(0);
    if (!child || !res)
	return res;

    QListViewItem *item = findLVItem(listView(), child);
    if (!item)
	return FALSE;
    listView()->setCurrentItem(item);
    return TRUE;
}*/

/*! \reimp */
bool QAccessibleListView::setSelected(int child, bool on, bool extend)
{
    if (!child || (extend &&
	listView()->selectionMode() != QListView::Extended &&
	listView()->selectionMode() != QListView::Multi))
	return FALSE;

    QListViewItem *item = findLVItem(listView(), child);
    if (!item)
	return FALSE;
    if (!extend) {
	listView()->setSelected(item, on);
    } else {
	QListViewItem *current = listView()->currentItem();
	if (!current)
	    return FALSE;
	bool down = item->itemPos() > current->itemPos();
	QListViewItemIterator it(current);
	while (it.current()) {
	    listView()->setSelected(it.current(), on);
	    if (it.current() == item)
		break;
	    if (down)
		++it;
	    else
		--it;
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleListView::clearSelection()
{
    listView()->clearSelection();
}

/*! \reimp */
QVector<int> QAccessibleListView::selection() const
{
    QVector<int> array;
    uint size = 0;
    int id = 1;
    array.resize(size);
    QListViewItemIterator it(listView());
    while (it.current()) {
	if (it.current()->isSelected()) {
	    ++size;
	    array.resize(size);
	    array[ (int)size-1 ] = id;
	}
	++it;
	++id;
    }
    return array;
}

#ifndef QT_NO_ICONVIEW
/*!
  \class QAccessibleIconView qaccessiblewidget.h
  \brief The QAccessibleIconView class implements the QAccessibleInterface for icon views.
*/

static QIconViewItem *findIVItem(QIconView *iconView, int child)
{
    int id = 1;
    QIconViewItem *item = iconView->firstItem();
    while (item && id < child) {
	item = item->nextItem();
	++id;
    }

    return item;
}

/*!
  Constructs a QAccessibleIconView object for \a o.
*/
QAccessibleIconView::QAccessibleIconView(QWidget *o)
    : QAccessibleScrollView(o, List)
{
    Q_ASSERT(widget()->inherits("QIconView"));
}

/*! Returns the icon view. */
QIconView *QAccessibleIconView::iconView() const
{
    return (QIconView*)widget();
}

/*! \reimp */
int QAccessibleIconView::itemAt(int x, int y) const
{
    QIconViewItem *item = iconView()->findItem(QPoint(x, y));
    return iconView()->index(item) + 1;
}

/*! \reimp */
QRect QAccessibleIconView::itemRect(int child) const
{
    QIconViewItem *item = findIVItem(iconView(), child);

    if (!item)
	return QRect();
    return item->rect();
}

/*! \reimp */
int QAccessibleIconView::itemCount() const
{
    return iconView()->count();
}

/*! \reimp */
QString QAccessibleIconView::text(Text t, int child) const
{
    if (!child || t != Name)
	return QAccessibleScrollView::text(t, child);

    QIconViewItem *item = findIVItem(iconView(), child);
    if (!item)
	return QString();
    return item->text();
}

/*! \reimp */
QAccessible::Role QAccessibleIconView::role(int child) const
{
    if (!child)
	return QAccessibleScrollView::role(child);
    return ListItem;
}

/*! \reimp */
int QAccessibleIconView::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QIconViewItem *item;
    if (!child || !(item = findIVItem(iconView(), child)))
	return state;

    if (item->isSelectable()) {
	if (iconView()->selectionMode() == QIconView::Multi)
	    state |= MultiSelectable;
	else if (iconView()->selectionMode() == QIconView::Extended)
	    state |= ExtSelectable;
	else if (iconView()->selectionMode() == QIconView::Single)
	    state |= Selectable;
	if (item->isSelected())
	    state |= Selected;
    }
    if (iconView()->itemsMovable())
	state |= Moveable;
    if (iconView()->focusPolicy() != QWidget::NoFocus) {
	state |= Focusable;
	if (item == iconView()->currentItem())
	    state |= Focused;
    }

    return state;
}

/*! \reimp
QAccessibleInterface *QAccessibleIconView::focusChild(int *child) const
{
    QIconViewItem *item = iconView()->currentItem();
    if (!item)
	return 0;

    *child = iconView()->index(item);
    return (QAccessibleInterface*)this;
}
*/
/*! \reimp
bool QAccessibleIconView::setFocus(int child)
{
    bool res = QAccessibleScrollView::setFocus(0);
    if (!child || !res)
	return res;

    QIconViewItem *item = findIVItem(iconView(), child);
    if (!item)
	return FALSE;
    iconView()->setCurrentItem(item);
    return TRUE;
}*/

/*! \reimp */
bool QAccessibleIconView::setSelected(int child, bool on, bool extend )
{
    if (!child || (extend &&
	iconView()->selectionMode() != QIconView::Extended &&
	iconView()->selectionMode() != QIconView::Multi))
	return FALSE;

    QIconViewItem *item = findIVItem(iconView(), child);
    if (!item)
	return FALSE;
    if (!extend) {
	iconView()->setSelected(item, on, TRUE);
    } else {
	QIconViewItem *current = iconView()->currentItem();
	if (!current)
	    return FALSE;
	bool down = FALSE;
	QIconViewItem *temp = current;
	while ((temp = temp->nextItem())) {
	    if (temp == item) {
		down = TRUE;
		break;
	    }
	}
	temp = current;
	if (down) {
	    while ((temp = temp->nextItem())) {
		iconView()->setSelected(temp, on, TRUE);
		if (temp == item)
		    break;
	    }
	} else {
	    while ((temp = temp->prevItem())) {
		iconView()->setSelected(temp, on, TRUE);
		if (temp == item)
		    break;
	    }
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleIconView::clearSelection()
{
    iconView()->clearSelection();
}

/*! \reimp */
QVector<int> QAccessibleIconView::selection() const
{
    QVector<int> array;
    uint size = 0;
    int id = 1;
    array.resize(iconView()->count());
    QIconViewItem *item = iconView()->firstItem();
    while (item) {
	if (item->isSelected()) {
	    ++size;
	    array[ (int)size-1 ] = id;
	}
	item = item->nextItem();
	++id;
    }
    array.resize(size);
    return array;
}
#endif


/*!
  \class QAccessibleTextEdit qaccessiblewidget.h
  \brief The QAccessibleTextEdit class implements the QAccessibleInterface for richtext editors.
*/

/*!
  Constructs a QAccessibleTextEdit object for \a o.
*/
QAccessibleTextEdit::QAccessibleTextEdit(QWidget *o)
: QAccessibleScrollView(o, Pane)
{
    Q_ASSERT(widget()->inherits("QTextEdit"));
}

/*! Returns the text edit. */
QTextEdit *QAccessibleTextEdit::textEdit() const
{

    return (QTextEdit*)widget();
}

/*! \reimp */
int QAccessibleTextEdit::itemAt(int x, int y) const
{
    int p;
    QPoint cp = textEdit()->viewportToContents(QPoint(x,y));
    textEdit()->charAt(cp , &p);
    return p + 1;
}

/*! \reimp */
QRect QAccessibleTextEdit::itemRect(int item) const
{
    QRect rect = textEdit()->paragraphRect(item - 1);
    if (!rect.isValid())
	return QRect();
    QPoint ntl = textEdit()->contentsToViewport(QPoint(rect.x(), rect.y()));
    return QRect(ntl.x(), ntl.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleTextEdit::itemCount() const
{
    return textEdit()->paragraphs();
}

/*! \reimp */
QString QAccessibleTextEdit::text(Text t, int child) const
{
    if (!child || t != Name)
	return QAccessibleScrollView::text(t, child);
    return textEdit()->text(child-1);
}

/*! \reimp */
QAccessible::Role QAccessibleTextEdit::role(int child) const
{
    if (child)
	return EditableText;
    return QAccessibleScrollView::role(child);
}
