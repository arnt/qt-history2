#include "complexwidgets.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qevent.h>
#include <qheader.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <private/qtitlebar_p.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

QString Q_GUI_EXPORT qacc_stripAmp(const QString &text);

/*!
  \class QAccessibleHeader qaccessiblewidget.h
  \brief The QAccessibleHeader class implements the QAccessibleInterface for header widgets.
*/

/*!
  Constructs a QAccessibleHeader object for \a w.
*/
QAccessibleHeader::QAccessibleHeader(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(header());
    addControllingSignal("clicked(int)");
}

/*! Returns the QHeader. */
QHeader *QAccessibleHeader::header() const
{
    return qt_cast<QHeader*>(object());
}

/*! \reimp */
QRect QAccessibleHeader::rect(int child) const
{
    QPoint zero = header()->mapToGlobal(QPoint(0, 0));
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
    QString str;

    if (child <= childCount()) {
        switch (t) {
        case Name:
            str = header()->label(child - 1);
            break;
        case Description:
            str = QToolTip::textFor(widget(), header()->sectionRect(child-1).center());
            break;
        case Help: {
            QPoint p(header()->sectionRect(child-1).center());
            QHelpEvent event(QEvent::AccessibleQueryHelp, p, header()->mapToGlobal(p));
            QApplication::sendEvent(widget(), &event);
            break; }
        }
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);;
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleHeader::role(int) const
{
    return (header()->orientation() == Qt::Horizontal) ? ColumnHeader : RowHeader;
}

/*! \reimp */
int QAccessibleHeader::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    int section = child ? child - 1 : -1;
    if (!header()->isClickEnabled(section))
        state |= Unavailable;
    else
        state |= Selectable;
    if (child && section == header()->sortIndicatorSection())
        state |= Selected;
    if (header()->isResizeEnabled(section))
        state |= Sizeable;
    if (child && header()->isMovingEnabled())
        state |= Moveable;
    return state;
}

/*!
  \class QAccessibleTabBar qaccessiblewidget.h
  \brief The QAccessibleTabBar class implements the QAccessibleInterface for tab bars.
*/

/*!
  Constructs a QAccessibleTabBar object for \a w.
*/
QAccessibleTabBar::QAccessibleTabBar(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(tabBar());
}

/*! Returns the QTabBar. */
QTabBar *QAccessibleTabBar::tabBar() const
{
    return qt_cast<QTabBar*>(object());
}

QButton *QAccessibleTabBar::button(int child) const
{
    if (child <= tabBar()->count())
        return 0;
    if (child - tabBar()->count() == 1)
        return qFindChild<QButton*>(tabBar(), "qt_left_btn");
    if (child - tabBar()->count() == 2)
        return qFindChild<QButton*>(tabBar(), "qt_right_btn");
    Q_ASSERT(false);
    return 0;
}

/*! \reimp */
QRect QAccessibleTabBar::rect(int child) const
{
    if (!child)
        return QAccessibleWidget::rect(0);

    QPoint tp = tabBar()->mapToGlobal(QPoint(0,0));
    QRect rec;
    if (child <= tabBar()->count()) {
        QTab *tab = tabBar()->tabAt(child - 1);
        rec = tab->rect();
    } else {
        QWidget *widget = button(child);
        rec = widget->geometry();
    }
    return QRect(tp.x() + rec.x(), tp.y() + rec.y(), rec.width(), rec.height());
}

/*! \reimp */
int QAccessibleTabBar::childCount() const
{
    // tabs + scroll buttons
    return tabBar()->count() + 2;
}

/*! \reimp */
QString QAccessibleTabBar::text(Text t, int child) const
{
    QString str;

    if (child > tabBar()->count()) {
        bool left = child - tabBar()->count() == 1;
        switch (t) {
        case Name:
            return left ? QTabBar::tr("Scroll Left") : QTabBar::tr("Scroll Right");
        }
    } else if (child > 0) {
        QTab *tab = tabBar()->tabAt(child - 1);
        switch (t) {
        case Name:
            return qacc_stripAmp(tab->text());
        }
    }

    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);;
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleTabBar::role(int child) const
{
    if (!child)
        return PageTabList;
    if (child > tabBar()->count())
        return PushButton;
    return PageTab;
}

/*! \reimp */
int QAccessibleTabBar::state(int child) const
{
    int st = QAccessibleWidget::state(0);

    if (!child)
        return st;

    if (child > tabBar()->count()) {
        QWidget *bt = button(child);
        if (!bt->isEnabled())
            st |= Unavailable;
        return st;
    }

    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab)
        return st;

    if (!tab->isEnabled())
        st |= Unavailable;
    else
        st |= Selectable;

    if (tabBar()->currentTab() == tab->identifier())
        st |= Selected;

    return st;
}

/*! \reimp */
bool QAccessibleTabBar::doAction(int, int child)
{
    if (!child)
        return false;

    if (child > tabBar()->count()) {
        QButton *bt = button(child);
        if (!bt->isEnabled())
            return false;
        bt->animateClick();
        return true;
    }
    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab || !tab->isEnabled())
        return false;
    tabBar()->setCurrentTab(tab);
    return true;
}

/*! \reimp */
bool QAccessibleTabBar::setSelected(int child, bool on, bool extend)
{
    if (!child || !on || extend || child > tabBar()->count())
        return false;

    QTab *tab = tabBar()->tabAt(child - 1);
    if (!tab || !tab->isEnabled())
        return false;
    tabBar()->setCurrentTab(tab);
    return true;
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
    case CurrentText:
        if (comboBox()->editable()) {
            tp = comboBox()->lineEdit()->mapToGlobal(QPoint(0,0));
            r = comboBox()->lineEdit()->rect();
        } else  {
            tp = comboBox()->mapToGlobal(QPoint(0,0));
            r = comboBox()->style().querySubControlMetrics(QStyle::CC_ComboBox, comboBox(), QStyle::SC_ComboBoxEditField);
        }
        break;
    case OpenList:
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
    if (entry > ComboBoxSelf) switch (rel) {
    case Child:
        if (entry < PopupList)
            return entry;
        if (entry == PopupList)
            return QAccessible::queryAccessibleInterface(comboBox()->listBox(), target) ? 0 : -1;
        break;
    case QAccessible::Left:
        return entry == OpenList ? CurrentText : -1;
    case QAccessible::Right:
        return entry == CurrentText ? OpenList : -1;
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
    return comboBox()->listBox() ? PopupList : OpenList;
}

/*! \reimp */
int QAccessibleComboBox::childAt(int x, int y) const
{
    QPoint gp = widget()->mapToGlobal(QPoint(0, 0));
    if (!QRect(gp.x(), gp.y(), widget()->width(), widget()->height()).contains(x, y))
        return -1;

    // a complex control
    for (int i = 1; i < PopupList; ++i) {
        if (rect(i).contains(x, y))
            return i;
    }
    return 0;
}

/*! \reimp */
int QAccessibleComboBox::indexOfChild(const QAccessibleInterface *child) const
{
    if (child->object() == comboBox()->listBox())
        return PopupList;
    return -1;
}

/*! \reimp */
QString QAccessibleComboBox::text(Text t, int child) const
{
    QString str;

    switch (t) {
    case Name:
        if (child == OpenList)
            str = QComboBox::tr("Open");
        else
            str = QAccessibleWidget::text(t, 0);
        break;
    case Accelerator:
        if (child == OpenList)
            str = (QString)QKeySequence(Key_Down);
    case Value:
        if (comboBox()->editable())
            str = comboBox()->lineEdit()->text();
        else
            str = comboBox()->currentText();
        break;
    default:
        break;
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, 0);
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleComboBox::role(int child) const
{
    switch (child) {
    case CurrentText:
        if (comboBox()->editable())
            return EditableText;
        return StaticText;
    case OpenList:
        return PushButton;
    case PopupList:
        return List;
    default:
        return ComboBox;
    }
}

/*! \reimp */
int QAccessibleComboBox::state(int /*child*/) const
{
    return QAccessibleWidget::state(0);
}

/*! \reimp */
bool QAccessibleComboBox::doAction(int, int child)
{
    if (child != 2)
        return false;
    comboBox()->popup();
    return true;
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
    if (str.size())
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
            return titleBar()->window()->windowTitle();
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
int QAccessibleTitleBar::state(int child) const
{
    return QAccessibleWidget::state(child);
}

/*! \reimp */
bool QAccessibleTitleBar::doAction(int, int child)
{
    switch (child) {
    case 3:
        if (titleBar()->window()->isMinimized())
            titleBar()->window()->showNormal();
        else
            titleBar()->window()->showMinimized();
        return true;
    case 4:
        if (titleBar()->window()->isMaximized())
            titleBar()->window()->showNormal();
        else
            titleBar()->window()->showMaximized();
        return true;
    case 5:
        titleBar()->window()->close();
        return true;
    default:
        break;
    }
    return false;
}
