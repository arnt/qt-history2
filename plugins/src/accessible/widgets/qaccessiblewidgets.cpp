#include "qaccessiblewidgets.h"

#include <qstyle.h>
#include <qgroupbox.h>
#include <qscrollview.h>
#include <qheader.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qtextedit.h>
#include <qwidgetstack.h>
#include <private/qtitlebar_p.h>


QString buddyString(QWidget *widget)
{
    QWidget *parent = widget->parentWidget();
    QObjectList ol = parent->queryList("QLabel", 0, FALSE, FALSE);
    if (!ol.count())
	return QString();

    QString str;

    QList<QObject*>::Iterator it = ol.begin();
    while (it != ol.end()) {
	QLabel *label = (QLabel*)*it;
	++it;
	if (label->buddy() == widget) {
	    str = label->text();
	    break;
	}
    }
    if (!!str)
	return str;

    if (parent->inherits("QGroupBox"))
	return ((QGroupBox*)parent)->title();

    return QString();
}

QString stripAmp(const QString &text)
{
    if (text.isEmpty())
	return text;

    QString n = text;
    for (uint i = 0; i < n.length(); i++) {
	if (n[(int)i] == '&')
	    n.remove(i, 1);
    }
    return n;
}

QString hotKey(const QString &text)
{
    if (text.isEmpty())
	return text;

    QString n = text;
    int fa = 0;
    bool ac = FALSE;
    while ((fa = n.find("&", fa)) != -1) {
	if (n.at(fa+1) != '&') {
	    ac = TRUE;
	    break;
	}
    }
    if (fa != -1 && ac)
	return QString(n.at(fa + 1));

    return QString();
}

/*!
  \class QAccessibleWidgetStack qaccessible.h
  \brief The QAccessibleWidgetStack class implements the QAccessibleInterface for widget stacks.
*/

/*!
  Creates a QAccessibleWidgetStack object for \a w.
*/
QAccessibleWidgetStack::QAccessibleWidgetStack(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(widgetStack());
}

/*! Returns the widget stack. */
QWidgetStack *QAccessibleWidgetStack::widgetStack() const
{
    return qt_cast<QWidgetStack*>(object());
}

/*! \reimp */
int QAccessibleWidgetStack::childAt(int, int) const
{
    return widgetStack()->id(widgetStack()->visibleWidget()) + 1;
}

/*!
  \class QAccessibleHeader qaccessiblewidget.h
  \brief The QAccessibleHeader class implements the QAccessibleInterface for header widgets.
*/

/*!
  Constructs a QAccessibleHeader object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleHeader::QAccessibleHeader(QWidget *o, QString description,
    QString value, QString help, QString defAction, QString accelerator)
: QAccessibleWidget(o, NoRole, QString(), description, value, help, SetFocus, defAction, accelerator)
{
    Q_ASSERT(header());
}

/*! Returns the QHeader. */
QHeader *QAccessibleHeader::header() const
{
    return qt_cast<QHeader*>(object());
}

/*! \reimp */
QRect QAccessibleHeader::rect(int child) const
{
    QPoint zero = header()->mapToGlobal(QPoint (0,0));
    QRect sect = header()->sectionRect(child - 1);
    return QRect(sect.x() + zero.x(), sect.y() + zero.y(), sect.width(), sect.height());
}

/*! \reimp */
int QAccessibleHeader::childCount() const
{
    return header()->count();
}

/*! \reimp */
QString QAccessibleHeader::text(Text t, int child) const
{
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;

    switch (t) {
    case Name:
	return header()->label(child - 1);
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleHeader::role(int child) const
{
    if (header()->orientation() == Qt::Horizontal)
	return child ? Column : ColumnHeader;
    else
	return child ? Row : RowHeader;
}

/*! \reimp */
QAccessible::State QAccessibleHeader::state(int child) const
{
    return QAccessibleWidget::state(child);
}

/*!
  \class QAccessibleTabBar qaccessiblewidget.h
  \brief The QAccessibleTabBar class implements the QAccessibleInterface for tab bars.
*/

/*!
  Constructs a QAccessibleTabBar object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleTabBar::QAccessibleTabBar(QWidget *o, QString description,
    QString value, QString help, QString defAction, QString accelerator)
    : QAccessibleWidget(o, NoRole, QString(), description, value, help, Select, defAction, accelerator)
{
    Q_ASSERT(tabBar());
}

/*! Returns the QTabBar. */
QTabBar *QAccessibleTabBar::tabBar() const
{
    return qt_cast<QTabBar*>(object());
}

/*! \reimp */
QRect QAccessibleTabBar::rect(int child) const
{
    if (!child)
	return QAccessibleWidget::rect(0);

    if (child > tabBar()->count()) {
	// get buttonLeft/buttonRight
	QRect r;
	return r;
    }

    QTab *tab = tabBar()->tabAt(child - 1);

    QPoint tp = tabBar()->mapToGlobal(QPoint(0,0));
    QRect rec = tab->rect();
    return QRect(tp.x() + rec.x(), tp.y() + rec.y(), rec.width(), rec.height());
}

/*! \reimp */
int QAccessibleTabBar::childCount() const
{
    int wc = QAccessibleWidget::childCount();
    wc += tabBar()->count();
    return wc;
}

/*! \reimp */
QString QAccessibleTabBar::text(Text t, int child) const
{
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;

    if (!child)
	return QAccessibleWidget::text(t, child);
/*
    if (child > tabBar()->count()) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild(child - tabBar()->count(), &iface);
	if (!iface)
	    return QAccessibleWidget::text(t, 0);
	return iface->text(t, 0);
    }
*/
    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab)
	return QAccessibleWidget::text(t, 0);

    switch (t) {
    case Name:
	return stripAmp(tab->text());
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleTabBar::role(int child) const
{
    if (!child)
	return PageTabList;
/*
    if (child > tabBar()->count()) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild(child - tabBar()->count(), &iface);
	if (!iface)
	    return QAccessibleWidget::role(0);
	return iface->role(0);
    }
*/
    return PageTab;
}

/*! \reimp */
QAccessible::State QAccessibleTabBar::state(int child) const
{
    int st = QAccessibleWidget::state(0);

    if (!child)
	return (State)st;
/*
    if (child > tabBar()->count()) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild(child - tabBar()->count(), &iface);
	if (!iface)
	    return (State)st;
	return iface->state(0);
    }
*/

    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab)
	return (State)st;

    if (!tab->isEnabled())
	st |= Unavailable;
    else
	st |= Selectable;

    if (tabBar()->currentTab() == tab->identifier())
	st |= Selected;

    return (State)st;
}

/*! \reimp */
bool QAccessibleTabBar::doAction(int action, int child)
{
    if (!child)
	return FALSE;
/*
    if (child > tabBar()->count()) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild(child - tabBar()->count(), &iface);
	if (!iface)
	    return FALSE;
	return iface->doAction(action, 0);
    }
*/
    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab || !tab->isEnabled())
	return FALSE;
    tabBar()->setCurrentTab(tab);
    return TRUE;
}

/*! \reimp */
bool QAccessibleTabBar::setSelected(int child, bool on, bool extend)
{
    if (!child || !on || extend || child > tabBar()->count())
	return FALSE;

    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab || !tab->isEnabled())
	return FALSE;
    tabBar()->setCurrentTab(tab);
    return TRUE;
}

/*! \reimp */
QVector<int> QAccessibleTabBar::selection() const
{
    QVector<int> array(1);
    array[0] = tabBar()->indexOf(tabBar()->currentTab()) + 1;

    return array;
}

/*!
  \class QAccessibleComboBox qaccessiblewidget.h
  \brief The QAccessibleComboBox class implements the QAccessibleInterface for editable and read-only combo boxes.
*/


/*!
  Constructs a QAccessibleComboBox object for \a w.
*/
QAccessibleComboBox::QAccessibleComboBox(QWidget *w)
: QAccessibleWidget(w, ComboBox)
{
    Q_ASSERT(comboBox());
}

/*!
  Returns the combo box.
*/
QComboBox *QAccessibleComboBox::comboBox() const
{
    return qt_cast<QComboBox*>(object());
}

/*! \reimp */
QRect QAccessibleComboBox::rect(int child) const
{
    QPoint tp;
    QRect r;

    switch(child) {
    case 1:
	if (comboBox()->editable()) {
	    tp = comboBox()->lineEdit()->mapToGlobal(QPoint(0,0));
	    r = comboBox()->lineEdit()->rect();
	} else  {
	    tp = comboBox()->mapToGlobal(QPoint(0,0));
	    r = comboBox()->style().querySubControlMetrics(QStyle::CC_ComboBox, comboBox(), QStyle::SC_ComboBoxEditField);
	}
	break;
    case 2:
	tp = comboBox()->mapToGlobal(QPoint(0,0));
	r = comboBox()->style().querySubControlMetrics(QStyle::CC_ComboBox, comboBox(), QStyle::SC_ComboBoxArrow);
	break;
    default:
	return QAccessibleWidget::rect(child);
    }
    return QRect(tp.x() + r.x(), tp.y() + r.y(), r.width(), r.height());
}

/*! \reimp */
int QAccessibleComboBox::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (entry) switch (rel) {
    case QAccessible::Left:
	return entry > 1 ? entry - 1 : -1;
    case QAccessible::Right:
	return entry < childCount() ? entry + 1 : -1;
    case QAccessible::Up:
	return -1;
    case QAccessible::Down:
	return -1;
    default:
	break;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
}

/*! \reimp */
int QAccessibleComboBox::childCount() const
{
    return 2;
}

/*! \reimp */
int QAccessibleComboBox::childAt(int x, int y) const
{
    QPoint gp = widget()->mapToGlobal(QPoint(0, 0));
    if (!QRect(gp.x(), gp.y(), widget()->width(), widget()->height()).contains(x, y))
	return -1;

    int ccount = childCount();

    // a complex control
    for (int i = 1; i <= ccount; ++i) {
	if (rect(i).contains(x, y))
	    return i;
    }
    return 0;
}

/*! \reimp */
QString QAccessibleComboBox::text(Text t, int child) const
{
    QString str;

    switch (t) {
    case Name:
	if (child < 2)
	    return stripAmp(buddyString(comboBox()));
	return QComboBox::tr("Open");
    case Accelerator:
	if (child < 2) {
	    str = hotKey(buddyString(comboBox()));
	    if (!!str)
		return "Alt + " + str;
	    return str;
	}
	return QComboBox::tr("Alt + Down Arrow");
    case Value:
	if (child < 2) {
	    if (comboBox()->editable())
		return comboBox()->lineEdit()->text();
	    return comboBox()->currentText();
	}
	break;
/*
    case DefaultAction:
	if (child == 2)
	    return QComboBox::tr("Open");
	break;
*/
    default:
	str = QAccessibleWidget::text(t, 0);
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleComboBox::role(int child) const
{
    switch (child) {
    case 0:
	return ComboBox;
    case 1:
	if (comboBox()->editable())
	    return EditableText;
	return StaticText;
    case 2:
	return PushButton;
    default:
	return List;
    }
}

/*! \reimp */
QAccessible::State QAccessibleComboBox::state(int /*child*/) const
{
    return QAccessibleWidget::state(0);
}

/*! \reimp */
bool QAccessibleComboBox::doAction(int action, int child)
{
    if (child != 2)
	return FALSE;
    comboBox()->popup();
    return TRUE;
}

/*!
  \class QAccessibleTitleBar qaccessiblewidget.h
  \brief The QAccessibleTitleBar class implements the QAccessibleInterface for title bars.
*/

/*!
  Constructs a QAccessibleComboBox object for \a w.
*/
QAccessibleTitleBar::QAccessibleTitleBar(QWidget *w)
: QAccessibleWidget(w, TitleBar)
{
    Q_ASSERT(titleBar());
}

/*!
  Returns the title bar.
*/
QTitleBar *QAccessibleTitleBar::titleBar() const
{
    return qt_cast<QTitleBar*>(object());
}

/*! \reimp */
QRect QAccessibleTitleBar::rect(int child) const
{
    if (!child)
	return QAccessibleWidget::rect(child);

    QRect r;
    switch (child) {
    case 1:
	r = titleBar()->style().querySubControlMetrics(QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarSysMenu);
	break;
    case 2:
	r = titleBar()->style().querySubControlMetrics(QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarLabel);
	break;
    case 3:
	r = titleBar()->style().querySubControlMetrics(QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarMinButton);
	break;
    case 4:
	r = titleBar()->style().querySubControlMetrics(QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarMaxButton);
	break;
    case 5:
	r = titleBar()->style().querySubControlMetrics(QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarCloseButton);
	break;
    default:
	break;
    }

    QPoint tp = titleBar()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + r.x(), tp.y() + r.y(), r.width(), r.height());
}

/*! \reimp *
int QAccessibleTitleBar::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
	return QAccessibleWidget::navigate(direction, startControl);

    switch (direction) {
    case NavFirstChild:
	return 1;
	break;
    case NavLastChild:
	return childCount();
	break;
    case NavNext:
    case NavRight:
	return startControl + 1 > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
	return startControl -1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }
    return -1;
}
*/

/*! \reimp */
int QAccessibleTitleBar::childCount() const
{
    if (!titleBar()->testWFlags(WStyle_SysMenu))
	return 0;
    int control = 3;
    if (titleBar()->testWFlags(WStyle_Minimize))
	++control;
    if (titleBar()->testWFlags(WStyle_Maximize))
	++control;
    return control;
}

/*! \reimp */
QString QAccessibleTitleBar::text(Text t, int child) const
{
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;

    QWidget *window = titleBar()->window();
    switch (t) {
    case Name:
	switch (child) {
	case 1:
	    return QTitleBar::tr("System");
	case 3:
	    if (window && window->isMinimized())
		return QTitleBar::tr("Restore up");
	    return QTitleBar::tr("Minimize");
	case 4:
	    if (window && window->isMaximized())
		return QTitleBar::tr("Restore down");
	    return QTitleBar::tr("Maximize");
	case 5:
	    return QTitleBar::tr("Close");
	default:
	    break;
	}
	break;
    case Value:
	if (!child || child == 2)
	    return titleBar()->window()->caption();
	break;
/*
    case DefaultAction:
	if (child > 2)
	    return QTitleBar::tr("Press");
	break;
*/
    case Description:
	switch (child) {
	case 1:
	    return QTitleBar::tr("Contains commands to manipulate the window");
	case 3:
	    if (window && window->isMinimized())
		return QTitleBar::tr("Puts a minimized back to normal");
	    return QTitleBar::tr("Moves the window out of the way");
	case 4:
	    if (window && window->isMaximized())
		return QTitleBar::tr("Puts a maximized window back to normal");
	    return QTitleBar::tr("Makes the window full screen");
	case 5:
	    return QTitleBar::tr("Closes the window");
	default:
	    return QTitleBar::tr("Displays the name of the window and contains controls to manipulate it");
	}
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleTitleBar::role(int child) const
{
    switch (child)
    {
    case 1:
    case 3:
    case 4:
    case 5:
	return PushButton;
    default:
	return TitleBar;
    }
}

/*! \reimp */
QAccessible::State QAccessibleTitleBar::state(int child) const
{
    return QAccessibleWidget::state(child);
}

/*! \reimp */
bool QAccessibleTitleBar::doAction(int action, int child)
{
    switch (child) {
    case 3:
	if (titleBar()->window()->isMinimized())
	    titleBar()->window()->showNormal();
	else
	    titleBar()->window()->showMinimized();
	return TRUE;
    case 4:
	if (titleBar()->window()->isMaximized())
	    titleBar()->window()->showNormal();
	else
	    titleBar()->window()->showMaximized();
	return TRUE;
    case 5:
	titleBar()->window()->close();
	return TRUE;
    default:
	break;
    }
    return FALSE;
}


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

QAccessible::State QAccessibleViewport::state(int child) const
{
    return scrollView()->state(child);
}

bool QAccessibleViewport::setSelected(int child, bool on, bool extend)
{
    return scrollView()->setSelected(child, on, extend);
}

void QAccessibleViewport::clearSelection()
{
    scrollView()->clearSelection();
}

QVector<int> QAccessibleViewport::selection() const
{
    return scrollView()->selection();
}

/*!
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
*/

/*!
  Constructs a QAccessibleScrollView object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollView::QAccessibleScrollView(QWidget *o, Role role, QString name,
    QString description, QString value, QString help, QString defAction, QString accelerator)
    : QAccessibleWidget(o, role, name, description, value, help, SetFocus, defAction, accelerator)
{
}

/*! \reimp */
QString QAccessibleScrollView::text(Text t, int child) const
{
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;
    switch (t) {
    case Name:
	return buddyString(widget());
    default:
	break;
    }

    return str;
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
QAccessible::State QAccessibleListBox::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QListBoxItem *item;
    if (!child || !(item = listBox()->item(child - 1)))
	return (State)state;

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

    return (State)state;
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
    : QAccessibleScrollView(o, Outline)
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
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleListView::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QListViewItem *item;
    if (!child || !(item = findLVItem(listView(), child)))
	return (State)state;

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
    return (State)state;
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
    : QAccessibleScrollView(o, Outline)
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
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleIconView::state(int child) const
{
    int state = QAccessibleScrollView::state(child);
    QIconViewItem *item;
    if (!child || !(item = findIVItem(iconView(), child)))
	return (State)state;

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

    return (State)state;
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
