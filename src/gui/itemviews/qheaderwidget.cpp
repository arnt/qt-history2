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

#include "qheaderwidget.h"
#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <private/qabstractitemmodel_p.h>
#include <private/qheaderview_p.h>

class QHeaderModel : public QAbstractTableModel
{
    friend class QHeaderWidgetItem;

public:
    QHeaderModel(Qt::Orientation orientation, int sections, QHeaderWidget *parent);
    ~QHeaderModel();

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool insertSections(int section, int count = 1);
    bool removeSections(int section, int count = 1);
    void setSectionCount(int sections);

    void setItem(int section, QHeaderWidgetItem *item);
    QHeaderWidgetItem *takeItem(int section);
    QHeaderWidgetItem *item(int section) const;
    void removeItem(QHeaderWidgetItem *item);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null);
    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, QVariant &value);
    
    int rowCount() const;
    int columnCount() const;

    void clear();
    void emitDataChanged(QHeaderWidgetItem *item);

private:
    QVector<QHeaderWidgetItem*> items;
    Qt::Orientation orientation;
};

QHeaderModel::QHeaderModel(Qt::Orientation orientation, int sections, QHeaderWidget *parent)
    : QAbstractTableModel(parent),
      orientation(orientation)
{
    setSectionCount(sections);
    setup();
}

QHeaderModel::~QHeaderModel()
{
    clear();
}

bool QHeaderModel::insertRows(int row, const QModelIndex &, int count)
{
    if (orientation == Qt::Vertical)
        return insertSections(row, count);
    return false;
}

bool QHeaderModel::insertColumns(int column, const QModelIndex &, int count)
{
    if (orientation == Qt::Horizontal)
        return insertSections(column, count);
    return false;
}

bool QHeaderModel::removeRows(int row, const QModelIndex &, int count)
{
    if (orientation == Qt::Vertical)
        return removeSections(row, count);
    return false;
}

bool QHeaderModel::removeColumns(int column, const QModelIndex &, int count)
{
    if (orientation == Qt::Horizontal)
        return removeSections(column, count);
    return false;
}


bool QHeaderModel::insertSections(int section, int count)
{
    items.insert(section, count, 0);
    return true;
}

bool QHeaderModel::removeSections(int section, int count)
{
    items.remove(section, count);
    return true;
}

void QHeaderModel::setSectionCount(int sections)
{
    items.resize(sections);
}

void QHeaderModel::setItem(int section, QHeaderWidgetItem *item)
{
    if (section >= 0 || section < items.count()) {
        delete items.at(section);
        items[section] = item;
    }
}

QHeaderWidgetItem *QHeaderModel::takeItem(int section)
{
    if (section >= 0 || section < items.count()) {
        QHeaderWidgetItem *item = items.at(section);
        items.remove(section);
        return item;
    }
    return 0;
}

QHeaderWidgetItem *QHeaderModel::item(int section) const
{
    if (section >= 0 || section < items.count())
        return items.at(section);
    return 0;
}

void QHeaderModel::removeItem(QHeaderWidgetItem *item)
{
    int i = items.indexOf(item);
    if (i != -1) {
        items.at(i)->model = 0;
        items.at(i)->view = 0;
        delete items.at(i);
        items[i] = 0;
    }
}

QModelIndex QHeaderModel::index(int, int, const QModelIndex &)
{
    return QModelIndex::Null;
}

QVariant QHeaderModel::data(const QModelIndex &, int) const
{
    return QVariant();
}

QVariant QHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != this->orientation || section < 0 || section >= items.count())
        return QVariant();
    if (items.at(section))
        return items.at(section)->data(role);
    return section;
}

bool QHeaderModel::setHeaderData(int section, Qt::Orientation orientation, int role, QVariant &value)
{
    if (orientation != this->orientation || section < 0 || section >= items.count())
        return false;
    items.at(section)->setData(role, value);
    emit headerDataChanged(orientation, section, section);
    return true;
}

int QHeaderModel::rowCount() const
{
    return orientation == Qt::Vertical ? items.count() : 1;
}

int QHeaderModel::columnCount() const
{
    return orientation == Qt::Horizontal ? items.count() : 1;
}

void QHeaderModel::clear()
{
    for (int i = 0; i < items.count(); ++i) {
        items.at(i)->model = 0;
        items.at(i)->view = 0;
        delete items.at(i);
        items[i] = 0;
    }
    emit reset();
}

void QHeaderModel::emitDataChanged(QHeaderWidgetItem *item)
{
    int sec = items.indexOf(item);
    emit headerDataChanged(orientation, sec, sec);
}

/*!
  \class QHeaderWidgetItem
  \brief The QHeaderWidgetItem class provides an item widget for use with the
  QHeaderWidget item view class.

  \ingroup model-view

  Header items are used to display header labels for headers provided by the
  QHeaderWidget class used by QTableWidget and QTreeWidget. Each item
  provides a generic text label with an optional icon, and supports standard
  user interface features such as tooltips, "What's This?" help, and status
  tips.

  Items can be made checkable by specifying the appropriate flags to
  setFlags(). You can determine if an item is already checkable by calling
  flags() and comparing the value returned against the values defined in
  \l{QAbstractItemModel::ItemFlags}. Checkable items can be checked with
  setChecked(), and the checked() function returns a value that can be
  compared with the values defined in \l{QCheckBox::ToggleState}.

  The appearance of header items can be customized in a variety of ways:

  \list
  \i The background color can be obtained with backgroundColor() and set
     with setBackgroundColor().
  \i The text color can be obtained with textColor() and set
     with setTextColor().
  \i The font used for the text can be obtained with font() and set
     with setFont().
  \i If the header item has been given an icon to use, this can be obtained
     with the icon() function and set with setIcon().
  \endlist

  Additionally, the item can provide tooltips, status tips, and "What's This?"
  help. The text used for each of these can be found with toolTip(),
  statusTip(), and whatsThis(). The text used for these features can be
  set with setToolTip(), setStatusTip(), and setWhatsThis().

  \sa QHeaderWidget QHeaderView
*/

/*!
    \fn QHeaderWidget *QHeaderWidgetItem::headerWidget() const

    Returns the header widget that contains the item.
*/

/*!
    \fn QAbstractItemModel::ItemFlags QHeaderWidgetItem::flags() const
*/

/*!
    \fn void QHeaderWidgetItem::setFlags()

    Sets the item flags for the item to \a flags (see
    \l{QAbstractItemModel::ItemFlags}).
*/

/*!
    \fn QString QHeaderWidgetItem::text() const

    Returns the text displayed in the item.
*/

/*!
    \fn void QHeaderWidgetItem::setText(const QString &text)

    Sets the text displayed in the item to the given \a text.

    \sa text() setIcon()
*/

/*!
    \fn QIcon QHeaderWidgetItem::icon() const

    Returns the icon displayed in the item.

    \sa setIcon() text()
*/

/*!
    \fn void QHeaderWidgetItem::setIcon(const QIcon &icon)

    Sets the icon displayed in the item to the given \a icon.

    \sa icon() setText()
*/

/*!
    \fn QString QHeaderWidgetItem::statusTip() const

    Returns the item's status tip text.

    \sa setStatusTip() toolTip() whatsThis()
*/

/*!
    \fn void QHeaderWidgetItem::setStatusTip(const QString &statusTip)

    Sets the status tip for the item to \a statusTip.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn QString QHeaderWidgetItem::toolTip() const

    Returns the item's tooltip text.

    \sa setToolTip() statusTip() whatsThis()
*/

/*!
    \fn void QHeaderWidgetItem::setToolTip(const QString &toolTip)

    Sets the tooltip for the item to \a toolTip.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn QString QHeaderWidgetItem::whatsThis() const

    Returns the item's "What's This?" text.

    \sa setWhatsThis() statusTip() toolTip()
*/

/*!
    \fn void QHeaderWidgetItem::setWhatsThis(const QString &whatsThis)

    Sets the "What's This?" help for this item to \a whatsThis.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
    \fn QFont QHeaderWidgetItem::font() const

    Returns the font used for the item's text.

    \sa setFont() textColor()
*/

/*!
    \fn void QHeaderWidgetItem::setFont(const QFont &font)

    Sets the font used for the item's text to the given \a font.

    \sa font() setTextColor()
*/

/*!
    \fn QColor QHeaderWidgetItem::backgroundColor() const

    Returns the background color of the item.

    \sa setBackgroundColor() textColor()
*/

/*!
    \fn void QHeaderWidgetItem::setBackgroundColor(const QColor &color)

    Sets the background color of the item to the given \a color.

    \sa backgroundColor() textColor()
*/

/*!
    \fn QColor QHeaderWidgetItem::textColor() const

    Returns the color used for the item's text.

    \sa setTextColor() backgroundColor() font()
*/

/*!
    \fn void QHeaderWidgetItem::setTextColor(const QColor &color)

    Sets the color used for the item's text to the given \a color.

    \sa color() setFont()
*/

/*!
    \fn int QHeaderWidgetItem::checked() const

    Returns the checked state of the item (see \l{QCheckBox::ToggleState}).

    Only checkable items can be checked. By default, items are not
    checkable.

    \sa flags()
*/

/*!
    \fn void QHeaderWidgetItem::setChecked(const bool checked)

    Checks the item if \a checked is true; otherwise the item
    will be shown as unchecked.

    \sa checked()
*/


/*!
  Constructs an empty header item.
*/

QHeaderWidgetItem::QHeaderWidgetItem()
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEnabled)
{
}

/*!
  Destroys the header item.
*/

QHeaderWidgetItem::~QHeaderWidgetItem()
{
    if (model) {
        int sec = model->items.indexOf(this);
        model->items[sec] = 0; // remove this
    }
}

/*!
  This function sets \a value for the given \a role (see
  \l{QAbstractItemModel::Role}). Reimplement this function if you need
  extra roles or special behavior for certain roles.
*/

void QHeaderWidgetItem::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            break;
        }
    }
    values.append(Data(role, value));
    if (model)
        model->emitDataChanged(this);
}

/*!
   This function returns the items data for the given \a role (see
   \l{QAbstractItemModel::Role}). Reimplement this function if you need
   extra roles or special behavior for certain roles.
*/

QVariant QHeaderWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

/*!
  Returns true if this items text is less then \a other items text.
*/

bool QHeaderWidgetItem::operator<(const QHeaderWidgetItem &other) const
{
    return text() < other.text();
}

/*!
  Removes all item data.
*/
void QHeaderWidgetItem::clear()
{
    values.clear();
    if (model)
        model->emitDataChanged(this);
}

#define d d_func()
#define q q_func()

// private

class QHeaderWidgetPrivate : public QHeaderViewPrivate
{
    Q_DECLARE_PUBLIC(QHeaderWidget)
public:
    QHeaderWidgetPrivate() : QHeaderViewPrivate() {}
    inline QHeaderModel *model() const { return ::qt_cast<QHeaderModel*>(q_func()->model()); }

    void emitClicked(int section, Qt::MouseButton button, Qt::KeyboarModifiers modifiers);
    void emitItemChanged(Qt::Orientation orientation, int first, int last);
};

void QHeaderWidgetPrivate::emitClicked(int section, Qt::MouseButton button,
                                       Qt::KeyboarModifiers modifiers)
{
    emit q->clicked(model()->item(section), button, modifiers);
}

void QHeaderWidgetPrivate::emitItemChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(orientation);
    if (first == last) // this should always be true
        emit q->itemChanged(model()->item(first));
    else
        qWarning("QHeaderWidgetPrivate: several items were changed");
    // Only one item at a time can change, so the warning should never be shown
}

// public

/*!
  \class QHeaderWidget qheaderwidget.h

  \brief The QHeaderWidget class provides a header for the model/view
  convenience view widgets.

  \ingroup model-view

  Header widgets are used by QTableWidget and QTreeWidget to display header
  labels for each row and column that they display.

  A header widget is constructed with a specific orientation and number of
  sections.
  A header that provides the column labels at the top of a table is
  constructed with a horizontal orientation; a header that provides the rows
  has a vertical orientation. The parent widget is usually a QTableWidget or
  a QTreeWidget:

  \code
      QHeaderWidget *columnsHeader = new QHeaderWidget(Qt::Horizontal, columns, tableWidget);
      QHeaderWidget *rowsHeader = new QHeaderWidget(Qt::Vertical, rows, tableWidget);
  \endcode

  For horizontal headers, each section corresponds to a column label which
  is provided by a \c QHeaderWidgetItem. For vertical headers, each section
  corresponds to a row label. 

  \sa QHeaderWidgetItem QTableWidget QTreeWidget \link model-view-programming.html
  Model/View Programming\endlink
*/

/*!
    \fn void QHeaderWidget::clicked(QHeaderWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)

    This signal is emitted when the specified \a item is clicked.
    The state of the mouse buttons is described by \a button; the
    \a modifiers reflect the state of the keyboard's modifier keys.

    The item may be 0 if the mouse was not clicked on an item.
*/

/*!
    \fn void QHeaderWidget::itemChanged(QHeaderWidgetItem *item)

    This signal is emitted when the specified \a item has changed.
*/

/*!
    Constructs a header view with the given \a orientation and \a parent widget.
*/

QHeaderWidget::QHeaderWidget(Qt::Orientation orientation, int sections, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setup(sections);
}

/*!
    Destroys the header widget and all its items.
*/

QHeaderWidget::~QHeaderWidget()
{
}

/*!
  Sets the number of sections in the header to \a sections.
*/

void QHeaderWidget::setSectionCount(int sections)
{
    d->model()->setSectionCount(sections);
}

/*!
  Returns the pointer to the item at \a section.
*/

QHeaderWidgetItem *QHeaderWidget::item(int section) const
{
    return d->model()->item(section);
}

/*!
  Sets the item at \a section to be \a item.
*/

void QHeaderWidget::setItem(int section, QHeaderWidgetItem *item)
{
    item->view = this;
    item->model = d->model();
    d->model()->setItem(section, item);
}

/*!
  Removes the item at \a section and returns it.
*/

QHeaderWidgetItem *QHeaderWidget::takeItem(int section)
{
    QHeaderWidgetItem *item = d->model()->takeItem(section);
    item->view = 0;
    item->model = 0;
    return item;
}

/*!
  Removes all items in the header.
*/

void QHeaderWidget::clear()
{
    d->model()->clear();
}

/*!
  \internal
*/

void QHeaderWidget::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
}

/*!
  \internal
*/

void QHeaderWidget::setup(int sections)
{
    setModel(new QHeaderModel(orientation(), sections, this));
    connect(this, SIGNAL(clicked(int,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitSectionClicked(int,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(model(), SIGNAL(headerDataChanged(Qt::orientation,int,int)),
            SLOT(emitItemChanged(Qt::orientation,int,int)));
}

#include "moc_qheaderwidget.cpp"
