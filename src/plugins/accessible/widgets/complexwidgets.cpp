/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include <qtableview.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <private/qtabbar_p.h>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QScrollBar>

#ifndef QT_NO_ACCESSIBILITY

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);

#ifndef QT_NO_ITEMVIEWS

QAccessibleItemRow::QAccessibleItemRow(QAbstractItemView *aView, const QModelIndex &index)
    : row(index), view(aView)
{
}

QRect QAccessibleItemRow::rect(int child) const
{
    if (!row.isValid() || !view)
        return QRect();

    QRect r;

    if (child) {
        r = view->visualRect(childIndex(child));
    } else {
        QModelIndex parent = row.parent();
        const int colCount = row.model()->columnCount(parent);
        for (int i = 0; i < colCount; ++i)
            r |= view->visualRect(row.model()->index(row.row(), i, parent));
    }

    return r.translated(view->viewport()->mapToGlobal(QPoint(0, 0)));
}

QString QAccessibleItemRow::text(Text t, int child) const
{
    if (!child)
        return QString();

    QModelIndex idx = childIndex(child);
    if (!idx.isValid())
        return QString();

    QString value;

    switch (t) {
    case Description:
        value = idx.model()->data(idx, Qt::AccessibleDescriptionRole).toString();
        break;
    case Value:
        value = idx.model()->data(idx, Qt::AccessibleTextRole).toString();
        if (value.isEmpty())
            value = idx.model()->data(idx, Qt::DisplayRole).toString();
        break;
    default:
        break;
    }
    return value;
}

void QAccessibleItemRow::setText(Text t, int child, const QString &text)
{
    if (!child)
        return;

    QModelIndex idx = childIndex(child);
    if (!idx.isValid())
        return;

    switch (t) {
    case Description:
        const_cast<QAbstractItemModel *>(idx.model())->setData(idx, text,
                                         Qt::AccessibleDescriptionRole);
        break;
    case Value:
        const_cast<QAbstractItemModel *>(idx.model())->setData(idx, text, Qt::EditRole);
        break;
    default:
        break;
    }
}

QModelIndex QAccessibleItemRow::childIndex(int child) const
{
    QList<QModelIndex> kids = children();
    Q_ASSERT(child >= 1 && child <= kids.count());
    return kids.at(child - 1);
}

QList<QModelIndex> QAccessibleItemRow::children() const
{
    QList<QModelIndex> kids;
    for (int i = 0; i < row.model()->columnCount(row.parent()); ++i) {
        QModelIndex idx = row.model()->index(row.row(), i, row.parent());
        if (!view->isIndexHidden(idx)) {
            kids << idx;
        }
    }
    return kids;
}

bool QAccessibleItemRow::isValid() const
{
    return row.isValid();
}

QObject *QAccessibleItemRow::object() const
{
    return 0;
}

int QAccessibleItemRow::childCount() const
{
    return children().count();
}

int QAccessibleItemRow::indexOfChild(const QAccessibleInterface *iface) const
{
    if (!iface || iface->role(0) != Row)
        return -1;

    QList<QModelIndex> kids = children();
    QModelIndex idx = static_cast<const QAccessibleItemRow *>(iface)->row;
    if (!idx.isValid())
        return -1;
    return kids.indexOf(idx) + 1;
}

QAccessible::Relation QAccessibleItemRow::relationTo(int child, const QAccessibleInterface *other,
        int otherChild) const
{
    if (!child && !otherChild && other->object() == view)
        return Child;
    if (!child && !otherChild && other == this)
        return Self;
    if (!child && otherChild && other == this)
        return Ancestor;
    if (child && otherChild && other == this)
        return Sibling;
    return Unrelated;
}

int QAccessibleItemRow::childAt(int x, int y) const
{
    if (!view || !view->isVisible())
        return -1;

    QModelIndex idx = view->indexAt(view->viewport()->mapFromGlobal(QPoint(x, y)));
    if (idx.isValid() && idx.parent() == row.parent() && idx.row() == row.row()) {
        QList<QModelIndex> kids = children();
        return kids.indexOf(idx) + 1;
    }

    return -1;
}

QAbstractItemView::CursorAction QAccessibleItemRow::toCursorAction(
                                           QAccessible::Relation rel)
{
    switch (rel) {
    case QAccessible::Up:
        return QAbstractItemView::MoveUp;
    case QAccessible::Down:
        return QAbstractItemView::MoveDown;
    case QAccessible::Left:
        return QAbstractItemView::MoveLeft;
    case QAccessible::Right:
        return QAbstractItemView::MoveRight;
    default:
        Q_ASSERT(false);
    }
    // should never be reached.
    return QAbstractItemView::MoveRight;
}

int QAccessibleItemRow::navigate(RelationFlag relation, int index,
                                 QAccessibleInterface **iface) const
{
    *iface = 0;
    if (!view)
        return -1;

    switch (relation) {
    case Ancestor: {
        if (!index)
            return -1;
        QAccessibleItemView *ancestor = new QAccessibleItemView(view, true);
        if (index == 1) {
            *iface = ancestor;
            return 1;
        }
        else if (index > 1)
            return ancestor->navigate(Ancestor, index - 1, iface);
        }
    case Child: {
        if (!index)
            return -1;
        QList<QModelIndex> kids = children();
        if (index < 1 && index > kids.count())
            return -1;

        return index;}
    case Sibling:
	if (index)
            return navigate(Child, index, iface);
        return -1;
    case Up:
    case Down:
    case Left:
    case Right: {
        // This is in the "not so nice" category. In order to find out which item
        // is geometrically around, we have to set the current index, navigate
        // and restore the index as well as the old selection
        view->setUpdatesEnabled(false);
        const QModelIndex oldIdx = view->currentIndex();
        QList<QModelIndex> kids = children();
        const QModelIndex currentIndex = index ? kids.at(index - 1) : QModelIndex(row);
        const QItemSelection oldSelection = view->selectionModel()->selection();
        view->setCurrentIndex(currentIndex);
        const QModelIndex idx = view->moveCursor(toCursorAction(relation), Qt::NoModifier);
        view->setCurrentIndex(oldIdx);
        view->selectionModel()->select(oldSelection, QItemSelectionModel::ClearAndSelect);
        view->setUpdatesEnabled(true);
        if (!idx.isValid())
            return -1;

        if (idx.parent() != row.parent() || idx.row() != row.row())
            *iface = new QAccessibleItemRow(view, idx);
        return index ? kids.indexOf(idx) + 1 : 0; }
    default:
        break;
    }

    return -1;
}

QAccessible::Role QAccessibleItemRow::role(int child) const
{
    if (!child)
        return Row;
    return Cell;
}

QAccessible::State QAccessibleItemRow::state(int child) const
{
    State st = Normal;

    if (!view)
        return st;

    QRect globalRect = view->viewport()->rect().translated(view->viewport()->mapToGlobal(QPoint(0,0)));
    if (!globalRect.intersects(rect(child))) {
        st |= Invisible;
    } else {
        if (child) {
            QModelIndex idx = childIndex(child);
            if (!idx.isValid())
                return st;

            if (view->selectionModel()->isSelected(idx))
                st |= Selected;
            if (idx.model()->data(idx, Qt::CheckStateRole).toInt() == Qt::Checked)
                st |= Checked;

            Qt::ItemFlags flags = idx.flags();
            if (flags & Qt::ItemIsSelectable) {
                st |= Selectable;
                if (view->selectionMode() == QAbstractItemView::MultiSelection)
                    st |= MultiSelectable;
                if (view->selectionMode() == QAbstractItemView::ExtendedSelection)
                    st |= ExtSelectable;
            }

        } else {
            if (view->selectionModel()->isRowSelected(row.row(), row.parent()))
                st |= Selected;
        }
    }


    return st;
}

int QAccessibleItemRow::userActionCount(int) const
{
    return 0;
}

QString QAccessibleItemRow::actionText(int, Text, int) const
{
    return QString();
}

static QItemSelection rowAt(const QModelIndex &idx)
{
    return QItemSelection(idx.sibling(idx.row(), 0),
                idx.sibling(idx.row(), idx.model()->columnCount(idx.parent())));
}

bool QAccessibleItemRow::doAction(int action, int child, const QVariantList & /*params*/)
{
    if (!view)
        return false;

    QModelIndex idx = child ? childIndex(child) : QModelIndex(row);
    if (!idx.isValid())
        return false;

    QItemSelectionModel::SelectionFlags command = QItemSelectionModel::NoUpdate;

    switch  (action) {
    case SetFocus:
        view->setCurrentIndex(idx);
        return true;
    case ExtendSelection:
        if (!child)
            return false;
        view->selectionModel()->select(QItemSelection(view->currentIndex(), idx),
                    QItemSelectionModel::SelectCurrent);
        return true;
    case Select:
        command = QItemSelectionModel::ClearAndSelect;
        break;
    case ClearSelection:
        command = QItemSelectionModel::Clear;
        break;
    case RemoveSelection:
        command = QItemSelectionModel::Deselect;
        break;
    case AddToSelection:
        command = QItemSelectionModel::SelectCurrent;
        break;
    }
    if (command == QItemSelectionModel::NoUpdate)
        return false;

    if (child)
        view->selectionModel()->select(idx, command);
    else
        view->selectionModel()->select(rowAt(row), command);
    return true;
}

QAccessibleItemView::QAccessibleItemView(QWidget *w, bool atViewport)
    : QAccessibleAbstractScrollArea(w), atViewport(atViewport)
{
    Q_ASSERT(itemView());
}

QAbstractItemView *QAccessibleItemView::itemView() const
{
    return qobject_cast<QAbstractItemView *>(object());
}

int QAccessibleItemView::indexOfChild(const QAccessibleInterface *iface) const
{
    if (atViewport) {
        if (!iface || iface->role(0) != Row)
            return -1;
        
        // ### This will fail if a row is hidden.
        QModelIndex idx = static_cast<const QAccessibleItemRow *>(iface)->row;
        if (!idx.isValid())
            return -1;

        return idx.row() + 1;
    } else {
        return QAccessibleAbstractScrollArea::indexOfChild(iface);
    }
}

QModelIndex QAccessibleItemView::childIndex(int child) const
{
    if (!atViewport)
        return QModelIndex();
    return itemView()->model()->index(child - 1, 0);
}

int QAccessibleItemView::childCount() const
{
    if (atViewport) {
        if (itemView()->model() == 0)
            return 0;
        return itemView()->model()->rowCount();
    } else {
        return QAccessibleAbstractScrollArea::childCount();
    }
}

QString QAccessibleItemView::text(Text t, int child) const
{
    if (atViewport) {
        if (!child)
            return QAccessibleAbstractScrollArea::text(t, child);

        QAccessibleItemRow item(itemView(), childIndex(child));
        return item.text(t, 1);
    } else {
        return QAccessibleAbstractScrollArea::text(t, child);        
    }
}

void QAccessibleItemView::setText(Text t, int child, const QString &text)
{
    if (atViewport) {
        if (!child) {
            QAccessibleAbstractScrollArea::setText(t, child, text);
            return;
        }

        QAccessibleItemRow item(itemView(), childIndex(child));
        item.setText(t, 1, text);
    } else {
        QAccessibleAbstractScrollArea::setText(t, child, text);
    }
}

QRect QAccessibleItemView::rect(int child) const
{
    if (atViewport) {
        QRect r;
        if (!child) {
            QWidget *w = itemView()->viewport();
            QPoint globalPos = w->mapToGlobal(QPoint(0,0));
            r = w->rect().translated(globalPos);
        } else {
            QModelIndex idx = childIndex(child);
            QAccessibleInterface *iface = new QAccessibleItemRow(itemView(), idx);
            r = iface->rect(0);
            delete iface;
        }
        return r;
    } else {
        return QAccessibleAbstractScrollArea::rect(child);
    }
}

QAccessible::Role QAccessibleItemView::role(int child) const
{
    if ((!atViewport && child) || (atViewport && child == 0)) {
        QAbstractItemView *view = itemView();
#ifndef QT_NO_TABLEVIEW
        if (qobject_cast<QTableView *>(view))
            return Table;
#endif
#ifndef QT_NO_LISTVIEW
        if (qobject_cast<QListView *>(view))
            return List;
#endif
        return Tree;
    }
    if (atViewport) {
        if (child)
            return Row;
    } else {
        return QAccessibleAbstractScrollArea::role(child);
    }
}

QAccessible::State QAccessibleItemView::state(int child) const
{
    if (atViewport) {
        if (!child)
            return QAccessibleAbstractScrollArea::state(child);

        QAccessibleItemRow item(itemView(), childIndex(child));
        return item.state(0);
    } else {
        return QAccessibleAbstractScrollArea::state(child);
    }
}

int QAccessibleItemView::navigate(RelationFlag relation, int index,
                                  QAccessibleInterface **iface) const
{
    if (atViewport) {
        if (relation == Ancestor && index == 1) {
            *iface = new QAccessibleItemView(itemView());
            return 0;
        } else {
            QModelIndex idx = childIndex(index);
            if (!idx.isValid()) {
                *iface = 0;
                return -1;
            }
            *iface = new QAccessibleItemRow(itemView(), idx);
            return 0;
        }
    } else {
        if (relation == Child && index == 1) {
            *iface = new QAccessibleItemView(itemView(), true);
            return 0;
        } else {
            return QAccessibleAbstractScrollArea::navigate(relation, index, iface);
        }
    }
}



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
    addControllingSignal(QLatin1String("sectionClicked(int)"));
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
            QAccessibleEvent event(QEvent::AccessibilityDescription, child);
            if (QApplication::sendEvent(widget(), &event))
                str = event.value();
            break; }
        case Help: {
            QAccessibleEvent event(QEvent::AccessibilityHelp, child);
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
#endif // QT_NO_ITEMVIEWS

#ifndef QT_NO_TABBAR
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
    if (child <= tabBar()->count() || !tabBar()->isVisible())
        return 0;
    QTabBarPrivate * const tabBarPrivate = tabBar()->d_func();
    if (child - tabBar()->count() == 1)
        return tabBarPrivate->leftB;
    if (child - tabBar()->count() == 2)
        return tabBarPrivate->rightB;
    Q_ASSERT(false);
    return 0;
}

/*! \reimp */
QRect QAccessibleTabBar::rect(int child) const
{
    if (!child || !tabBar()->isVisible())
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
    if (!tabBar()->isVisible())
        return 0;
    return tabBar()->count() + 2;
}

/*! \reimp */
QString QAccessibleTabBar::text(Text t, int child) const
{
    QString str;
    if (!tabBar()->isVisible())
        return str;

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
        if (!bt)
            return st;
        if (bt->isEnabled() == false)
            st |= Unavailable;
        if (bt->isVisible() == false)
            st |= Invisible;
        if (bt->focusPolicy() != Qt::NoFocus && bt->isActiveWindow())
            st |= Focusable;
        if (bt->hasFocus())
            st |= Focused;
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
bool QAccessibleTabBar::doAction(int action, int child, const QVariantList &)
{
    if (!child || !tabBar()->isVisible())
        return false;

    if (action != QAccessible::DefaultAction && action != QAccessible::Press)
        return false;

    if (child > tabBar()->count()) {
        QAbstractButton *bt = button(child);
        if (!bt->isEnabled() || !bt->isVisible())
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

#endif // QT_NO_TABBAR

#ifndef QT_NO_COMBOBOX
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
    if (!comboBox()->isVisible())
        return -1;
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
#ifndef QT_NO_SHORTCUT
    case Accelerator:
        if (child == OpenList)
            str = (QString)QKeySequence(Qt::Key_Down);
        // missing break?
#endif
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
#endif // QT_NO_COMBOBOX

static inline void removeInvisibleWidgetsFromList(QWidgetList *list)
{
    if (!list || list->isEmpty())
        return;

    for (int i = 0; i < list->count(); ++i) {
        QWidget *widget = list->at(i);
        if (!widget->isVisible())
            list->removeAt(i);
    }
}

#ifndef QT_NO_SCROLLAREA
// ======================= QAccessibleAbstractScrollArea =======================
QAccessibleAbstractScrollArea::QAccessibleAbstractScrollArea(QWidget *widget)
    : QAccessibleWidgetEx(widget, Client)
{
    Q_ASSERT(qobject_cast<QAbstractScrollArea *>(widget));
}

QString QAccessibleAbstractScrollArea::text(Text textType, int child) const
{
    if (!abstractScrollArea()->isVisible())
        return QString();
    if (child == Self)
        return QAccessibleWidgetEx::text(textType, 0);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QString();
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return QString();
    QString string = childInterface->text(textType, 0);
    delete childInterface;
    return string;
}

void QAccessibleAbstractScrollArea::setText(Text textType, int child, const QString &text)
{
    if (!abstractScrollArea()->isVisible() || text.isEmpty())
        return;
    if (child == 0) {
        QAccessibleWidgetEx::setText(textType, 0, text);
        return;
    }
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return;
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return;
    childInterface->setText(textType, 0, text);
    delete childInterface;
}

QAccessible::State QAccessibleAbstractScrollArea::state(int child) const
{
    if (child == Self)
        return QAccessibleWidgetEx::state(child);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QAccessibleWidgetEx::state(Self);
    QAccessibleInterface *childInterface = queryAccessibleInterface(children.at(child - 1));
    if (!childInterface)
        return QAccessibleWidgetEx::state(Self);
    QAccessible::State returnState = childInterface->state(0);
    delete childInterface;
    return returnState;
}

QVariant QAccessibleAbstractScrollArea::invokeMethodEx(QAccessible::Method, int, const QVariantList &)
{
    return QVariant();
}

int QAccessibleAbstractScrollArea::childCount() const
{
    if (!abstractScrollArea()->isVisible())
        return 0;
    return accessibleChildren().count();
}

int QAccessibleAbstractScrollArea::indexOfChild(const QAccessibleInterface *child) const
{
    if (!child || !child->object() || !abstractScrollArea()->isVisible())
        return -1;
    int index = accessibleChildren().indexOf(qobject_cast<QWidget *>(child->object()));
    if (index >= 0)
        return ++index;
    return -1;
}

int QAccessibleAbstractScrollArea::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
    if (!abstractScrollArea()->isVisible() || !target)
        return -1;

    *target = 0;
    QWidgetList children = accessibleChildren();
    if (entry < 0 || entry > children.count())
        return -1;

    QWidget *targetWidget = 0;
    QWidget *entryWidget = 0;
    if (entry == 0)
        entryWidget = abstractScrollArea();
    else if (entry > 0)
        entryWidget = children.at(entry - 1);
    AbstractScrollAreaElement entryElement = elementType(entryWidget);

    // Widgets to the left for the horizontal scrollBar.
    QWidgetList leftScrollBarWidgets = abstractScrollArea()
        ->scrollBarWidgets(isLeftToRight() ? Qt::AlignLeft : Qt::AlignRight);
    removeInvisibleWidgetsFromList(&leftScrollBarWidgets);

    // Widgets to the right for the horizontal scrollBar.
    QWidgetList rightScrollBarWidgets = abstractScrollArea()
        ->scrollBarWidgets(isLeftToRight() ? Qt::AlignRight : Qt::AlignLeft);
    removeInvisibleWidgetsFromList(&rightScrollBarWidgets);

    // Widgets above the vertical scrollBar.
    QWidgetList topScrollBarWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop);
    removeInvisibleWidgetsFromList(&topScrollBarWidgets);

    // Widgets below the vertical scrollBar.
    QWidgetList bottomScrollBarWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignBottom);
    removeInvisibleWidgetsFromList(&bottomScrollBarWidgets);

    // Not one of the most beautiful switches I've ever seen, but I believe it has
    // to be like this since each case need special handling.
    // It might be possible to make it more general, but I'll leave that as an exercise
    // to the reader. :-)
    switch (relation) {
    case Child:
        if (entry > 0)
            targetWidget = children.at(entry - 1);
        break;
    case Left:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            if (!isLeftToRight())
                targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        case HorizontalScrollBar:
            if (leftScrollBarWidgets.count() >= 1)
                targetWidget = leftScrollBarWidgets.back();
            else if (!isLeftToRight())
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        case VerticalScrollBar:
        case VerticalScrollBarWidget:
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->viewport();
            break;
        case CornerWidget:
            if (!isLeftToRight())
                break;
            if (rightScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.back();
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case HorizontalScrollBarWidget: {
            int index = leftScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index > 0)
                    targetWidget = leftScrollBarWidgets.at(index - 1);
                else if (!isLeftToRight())
                    targetWidget = abstractScrollArea()->cornerWidget();
                break;
            }
            index = rightScrollBarWidgets.indexOf(entryWidget);
            if (index > 0)
                targetWidget = rightScrollBarWidgets.at(index - 1);
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        }
        default:
            break;
        }
        break;
    case Right:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        case HorizontalScrollBar:
            if (rightScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.at(0);
            else
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        case VerticalScrollBar:
        case VerticalScrollBarWidget:
            if (!isLeftToRight())
                targetWidget = abstractScrollArea()->viewport();
            break;
        case CornerWidget:
            if (isLeftToRight())
                break;
            if (leftScrollBarWidgets.count() >= 1)
                targetWidget = rightScrollBarWidgets.at(0);
            else
                targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case HorizontalScrollBarWidget: {
            int index = leftScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < leftScrollBarWidgets.count() - 1)
                    targetWidget = leftScrollBarWidgets.at(index + 1);
                break;
            }
            index = rightScrollBarWidgets.indexOf(entryWidget);
            if (index != -1 && index < rightScrollBarWidgets.count() - 1) {
                targetWidget = rightScrollBarWidgets.at(index + 1);
                break;
            }
            if (isLeftToRight())
                targetWidget = abstractScrollArea()->cornerWidget();
            break;
        }
        default:
            break;
        }
        break;
    case Up:
        if (entry < 1)
            break;
        switch (entryElement) {
        case HorizontalScrollBar:
        case HorizontalScrollBarWidget:
            targetWidget = abstractScrollArea()->viewport();
            break;
        case VerticalScrollBar: {
            if (topScrollBarWidgets.count() >= 1)
                targetWidget = topScrollBarWidgets.back();
            break;
        }
        case VerticalScrollBarWidget: {
            int index = topScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index > 0)
                    targetWidget = topScrollBarWidgets.at(index - 1);
                break;
            }
            index = bottomScrollBarWidgets.indexOf(entryWidget);
            if (index > 0) {
                targetWidget = bottomScrollBarWidgets.at(index - 1);
                break;
            }
            targetWidget = abstractScrollArea()->verticalScrollBar();
            break;
        }
        case CornerWidget:
            if (bottomScrollBarWidgets.count() >= 1)
                targetWidget = bottomScrollBarWidgets.back();
            break;
        default:
            break;
        }
        break;
    case Down:
        if (entry < 1)
            break;
        switch (entryElement) {
        case Viewport:
            targetWidget = abstractScrollArea()->horizontalScrollBar();
            break;
        case VerticalScrollBar:
            if (bottomScrollBarWidgets.count() >= 1)
                targetWidget = bottomScrollBarWidgets.at(0);
            break;
        case VerticalScrollBarWidget: {
            int index = topScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < topScrollBarWidgets.count() - 1)
                    targetWidget = topScrollBarWidgets.at(index + 1);
                else
                    targetWidget = abstractScrollArea()->verticalScrollBar();
                break;
            }
            index = bottomScrollBarWidgets.indexOf(entryWidget);
            if (index != -1) {
                if (index < bottomScrollBarWidgets.count() - 1)
                    targetWidget = bottomScrollBarWidgets.at(index + 1);
                break;
            }
            targetWidget = abstractScrollArea()->cornerWidget();
            break;
        }
        default:
            break;
        }
        break;
    default:
        return QAccessibleWidgetEx::navigate(relation, entry, target);
    }

    if (targetWidget && !targetWidget->isVisible())
        return -1;
    *target = QAccessible::queryAccessibleInterface(targetWidget);
    return indexOfChild(*target);
}

QRect QAccessibleAbstractScrollArea::rect(int child) const
{
    if (!abstractScrollArea()->isVisible())
        return QRect();
    if (child == Self)
        return QAccessibleWidgetEx::rect(child);
    QWidgetList children = accessibleChildren();
    if (child < 1 || child > children.count())
        return QRect();
    const QWidget *childWidget = children.at(child - 1);
    if (!childWidget->isVisible())
        return QRect();
    return QRect(childWidget->mapToGlobal(QPoint(0, 0)), childWidget->size());
}

int QAccessibleAbstractScrollArea::childAt(int x, int y) const
{
    if (!abstractScrollArea()->isVisible())
        return -1;
    const QRect globalSelfGeometry = rect(Self);
    if (!globalSelfGeometry.isValid() || !globalSelfGeometry.contains(QPoint(x, y)))
        return -1;
    const QWidgetList children = accessibleChildren();
    for (int i = 0; i < children.count(); ++i) {
        const QWidget *child = children.at(i);
        const QRect globalChildGeometry = QRect(child->mapToGlobal(QPoint(0, 0)), child->size());
        if (globalChildGeometry.contains(QPoint(x, y))) {
            return ++i;
        }
    }
    return 0;
}

QAbstractScrollArea *QAccessibleAbstractScrollArea::abstractScrollArea() const
{
    return static_cast<QAbstractScrollArea *>(object());
}

QWidgetList QAccessibleAbstractScrollArea::accessibleChildren() const
{
    QWidgetList children;

    // Viewport.
    QWidget * viewport = abstractScrollArea()->viewport();
    if (viewport && viewport->isVisible())
        children.append(viewport);

    // Horizontal scrollBar and all its scrollWidgets.
    QScrollBar *horizontalScrollBar = abstractScrollArea()->horizontalScrollBar();
    if (horizontalScrollBar && horizontalScrollBar->isVisible()) {
        children.append(horizontalScrollBar);
        QWidgetList scrollWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignLeft | Qt::AlignRight);
        foreach (QWidget *widget, scrollWidgets) {
            if (widget->isVisible())
                children.append(widget);
        }
    }

    // Vertical scrollBar and all its scrollWidgets.
    QScrollBar *verticalScrollBar = abstractScrollArea()->verticalScrollBar();
    if (verticalScrollBar && verticalScrollBar->isVisible()) {
        children.append(verticalScrollBar);
        QWidgetList scrollWidgets = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop | Qt::AlignBottom);
        foreach (QWidget *widget, scrollWidgets) {
            if (widget->isVisible())
                children.append(widget);
        }
    }

    // CornerWidget.
    QWidget *cornerWidget = abstractScrollArea()->cornerWidget();
    if (cornerWidget && cornerWidget->isVisible())
        children.append(cornerWidget);

    return children;
}

QAccessibleAbstractScrollArea::AbstractScrollAreaElement
QAccessibleAbstractScrollArea::elementType(QWidget *widget) const
{
    if (!widget)
        return Undefined;

    if (widget == abstractScrollArea())
        return Self;
    if (widget == abstractScrollArea()->viewport())
        return Viewport;
    if (widget == abstractScrollArea()->horizontalScrollBar())
        return HorizontalScrollBar;
    if (widget == abstractScrollArea()->verticalScrollBar())
        return VerticalScrollBar;
    if (widget == abstractScrollArea()->cornerWidget())
        return CornerWidget;

    // Check horizontal scrollBar widgets.
    QWidgetList list = abstractScrollArea()->scrollBarWidgets(Qt::AlignLeft | Qt::AlignRight);
    foreach (QWidget *candidate, list) {
        if (widget == candidate)
            return HorizontalScrollBarWidget;
    }

    // Check vertical scrollBar widgets.
    list = abstractScrollArea()->scrollBarWidgets(Qt::AlignTop | Qt::AlignBottom);
    foreach (QWidget *candidate, list) {
        if (widget == candidate)
            return VerticalScrollBarWidget;
    }

    return Undefined;
}

bool QAccessibleAbstractScrollArea::isLeftToRight() const
{
    return abstractScrollArea()->isLeftToRight();
}

// ======================= QAccessibleScrollArea ===========================
QAccessibleScrollArea::QAccessibleScrollArea(QWidget *widget)
    : QAccessibleAbstractScrollArea(widget)
{
    Q_ASSERT(qobject_cast<QScrollArea *>(widget));
}
#endif // QT_NO_SCROLLAREA

#endif // QT_NO_ACCESSIBILITY
