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

#include "complexwidgets.h"

#include <qapplication.h>
#include <qabstractbutton.h>
#include <qevent.h>
#include <qheaderview.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);

/*!
  \class QAccessibleHeader qaccessiblewidget.h
  \brief The QAccessibleHeader class implements the QAccessibleInterface for header widgets.
  \internal

  \ingroup accessibility
*/

/*!
  Constructs a QAccessibleHeader object for \a w.
*/
QAccessibleHeader::QAccessibleHeader(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(header());
    addControllingSignal("sectionClicked(int, Qt::ButtonState)");
}

/*! Returns the QHeaderView. */
QHeaderView *QAccessibleHeader::header() const
{
    return qobject_cast<QHeaderView*>(object());
}

/*! \reimp */
QRect QAccessibleHeader::rect(int child) const
{
    QHeaderView *h = header();
    QPoint zero = h->mapToGlobal(QPoint(0, 0));
    int sectionSize = h->sectionSize(child - 1);
    int sectionPos = h->sectionPosition(child - 1);
    return h->orientation() == Qt::Horizontal
        ? QRect(zero.x() + sectionPos, zero.y(), sectionSize, h->height())
        : QRect(zero.x(), zero.y() + sectionPos, h->width(), sectionSize);
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
            str = header()->model()->headerData(child - 1, header()->orientation()).toString();
            break;
        case Description: {
            QAccessibleEvent event(QAccessibleEvent::Description, child);
            if (QApplication::sendEvent(widget(), &event))
                str = event.value();
            break; }
        case Help: {
            QAccessibleEvent event(QAccessibleEvent::Help, child);
            if (QApplication::sendEvent(widget(), &event))
                str = event.value();
            break; }
        default:
            break;
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
QAccessible::State QAccessibleHeader::state(int child) const
{
    State state = QAccessibleWidget::state(child);

    int section = child ? child - 1 : -1;
    if (header()->isSectionHidden(section))
        state |= Invisible;
    if (!header()->isClickable())
        state |= Unavailable;
    if (header()->resizeMode(section) != QHeaderView::Custom)
        state |= Sizeable;
    if (child && header()->isMovable())
        state |= Movable;
    return state;
}

/*!
  \class QAccessibleTabBar qaccessiblewidget.h
  \brief The QAccessibleTabBar class implements the QAccessibleInterface for tab bars.
  \internal

  \ingroup accessibility
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
    return qobject_cast<QTabBar*>(object());
}

QAbstractButton *QAccessibleTabBar::button(int child) const
{
    if (child <= tabBar()->count())
        return 0;
    if (child - tabBar()->count() == 1)
        return qFindChild<QAbstractButton*>(tabBar(), "qt_left_btn");
    if (child - tabBar()->count() == 2)
        return qFindChild<QAbstractButton*>(tabBar(), "qt_right_btn");
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
        rec = tabBar()->tabRect(child - 1);
    } else {
        QWidget *widget = button(child);
        rec = widget ? widget->geometry() : QRect();
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
        default:
            break;
        }
    } else if (child > 0) {
        switch (t) {
        case Name:
            return qt_accStripAmp(tabBar()->tabText(child - 1));
        default:
            break;
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
QAccessible::State QAccessibleTabBar::state(int child) const
{
    State st = QAccessibleWidget::state(0);

    if (!child)
        return st;

    QTabBar *tb = tabBar();

    if (child > tb->count()) {
        QWidget *bt = button(child);
        if (bt && !bt->isEnabled())
            st |= Unavailable;
        return st;
    }

    if (!tb->isTabEnabled(child - 1))
        st |= Unavailable;
    else
        st |= Selectable;

    if (!tb->currentIndex() == child - 1)
        st |= Selected;

    return st;
}

/*! \reimp */
bool QAccessibleTabBar::doAction(int, int child, const QVariantList &)
{
    if (!child)
        return false;

    if (child > tabBar()->count()) {
        QAbstractButton *bt = button(child);
        if (!bt->isEnabled())
            return false;
        bt->animateClick();
        return true;
    }
    if (!tabBar()->isTabEnabled(child - 1))
        return false;
    tabBar()->setCurrentIndex(child - 1);
    return true;
}

/*!
    Selects the item with index \a child if \a on is true; otherwise
    unselects it. If \a extend is true and the selection mode is not
    \c Single and there is an existing selection, the selection is
    extended to include all the items from the existing selection up
    to and including the item with index \a child. Returns true if a
    selection was made or extended; otherwise returns false.

    \sa selection() clearSelection()
*/
bool QAccessibleTabBar::setSelected(int child, bool on, bool extend)
{
    if (!child || !on || extend || child > tabBar()->count())
        return false;

    if (!tabBar()->isTabEnabled(child - 1))
        return false;
    tabBar()->setCurrentIndex(child - 1);
    return true;
}

/*!
    Returns a (possibly empty) list of indexes of the items selected
    in the list box.

    \sa setSelected() clearSelection()
*/
QVector<int> QAccessibleTabBar::selection() const
{
    QVector<int> array;
    if (tabBar()->currentIndex() != -1)
        array +=tabBar()->currentIndex() + 1;
    return array;
}

/*!
  \class QAccessibleComboBox qaccessiblewidget.h
  \brief The QAccessibleComboBox class implements the QAccessibleInterface for editable and read-only combo boxes.
  \internal

  \ingroup accessibility
*/

/*!
    \enum QAccessibleComboBox::ComboBoxElements

    \internal

    \value ComboBoxSelf
    \value CurrentText
    \value OpenList
    \value PopupList
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
  Returns the combobox.
*/
QComboBox *QAccessibleComboBox::comboBox() const
{
    return qobject_cast<QComboBox*>(object());
}

/*! \reimp */
QRect QAccessibleComboBox::rect(int child) const
{
    QPoint tp;
    QStyle::SubControl sc;
    QRect r;
    switch(child) {
    case CurrentText:
        if (comboBox()->isEditable()) {
            tp = comboBox()->lineEdit()->mapToGlobal(QPoint(0,0));
            r = comboBox()->lineEdit()->rect();
            sc = QStyle::SC_None;
        } else  {
            tp = comboBox()->mapToGlobal(QPoint(0,0));
            sc = QStyle::SC_ComboBoxEditField;
        }
        break;
    case OpenList:
        tp = comboBox()->mapToGlobal(QPoint(0,0));
        sc = QStyle::SC_ComboBoxArrow;
        break;
    default:
        return QAccessibleWidget::rect(child);
    }

    if (sc != QStyle::SC_None) {
        QStyleOptionComboBox option;
        r = comboBox()->style()->subControlRect(QStyle::CC_ComboBox, &option, sc, comboBox());
    }
    return QRect(tp.x() + r.x(), tp.y() + r.y(), r.width(), r.height());
}

/*! \reimp */
int QAccessibleComboBox::navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (entry > ComboBoxSelf) switch (rel) {
    case Child:
        if (entry < PopupList)
            return entry;
        if (entry == PopupList) {
            *target = QAccessible::queryAccessibleInterface(comboBox()->view());
            return *target ? 0 : -1;
        }
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
    return comboBox()->view() ? PopupList : OpenList;
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
    if (child->object() == comboBox()->view())
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
            str = (QString)QKeySequence(Qt::Key_Down);
    case Value:
        if (comboBox()->isEditable())
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
        if (comboBox()->isEditable())
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
QAccessible::State QAccessibleComboBox::state(int /*child*/) const
{
    return QAccessibleWidget::state(0);
}

/*! \reimp */
bool QAccessibleComboBox::doAction(int, int child, const QVariantList &)
{
    if (child != 2)
        return false;
    comboBox()->showPopup();
    return true;
}

