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

#include "qstackedlayout.h"
#include "qlayout_p.h"

#include <qlist.h>
#include <qwidget.h>
#include "private/qlayoutengine_p.h"

class QStackedLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QStackedLayout)
public:
    QStackedLayoutPrivate() : index(-1) {}
    QList<QLayoutItem *> list;
    int index;
};

/*!
    \class QStackedLayout

    \brief The QStackedLayout class provides a stack of widgets where
    only one widget is visible at a time.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QStackedLayout can be used to create a user interface similar to
    the one provided by QTabWidget. There is also a convenience
    QStackedWidget class built on top of QStackedLayout.

    A QStackedLayout can be populated with a number of child widgets
    ("pages"). For example:

    \quotefromfile snippets/qstackedlayout/main.cpp
    \skipto firstPageWidget
    \printto QComboBox

    \skipto QVBoxLayout
    \printline QVBoxLayout
    \skipto mainLayout->addLayout
    \printuntil setLayout

    QStackedLayout provides no intrinsic means for the user to switch
    page. This is typically done through a QComboBox or a QListWidget
    that stores the titles of the QStackedLayout's pages. For
    example:

    \quotefromfile snippets/qstackedlayout/main.cpp
    \skipto QComboBox
    \printuntil SLOT

    When populating a layout, the widgets are added to an internal
    list. The indexOf() function returns the index of a widget in that
    list. The widgets can either be added to the end of the list using
    the addWidget() function, or inserted at a given index using the
    insertWidget() function. The removeWidget() function removes the
    widget at the given index from the layout. The number of widgets
    contained in the layout, can be obtained using the count()
    function.

    The widget() function returns the widget at a given index
    position. The index of the widget that is shown on screen is given
    by currentIndex() and can be changed using setCurrentIndex(). In a
    similar manner, the currently shown widget can be retrieved using
    the currentWidget() function, and altered using the
    setCurrentWidget() function.

    Whenever the current widget in the layout changes or a widget is
    removed from the layout, the currentChanged() and widgetRemoved()
    signals are emitted respectively.

    \sa QStackedWidget, QTabWidget
*/

/*!
    \fn void QStackedLayout::currentChanged(int index)

    This signal is emitted whenever the current widget in the layout
    changes.  The \a index specifies the index of the new current
    widget.

    \sa currentWidget(), setCurrentWidget()
*/

/*!
    \fn void QStackedLayout::widgetRemoved(int index)

    This signal is emitted whenever a widget is removed from the
    layout. The widget's \a index is passed as parameter.

    \sa removeWidget()
*/

/*!
    Constructs a QStackedLayout with no parent.

    This QStackedLayout must be installed on a widget later on to
    become effective.

    \sa addWidget(), insertWidget()
*/
QStackedLayout::QStackedLayout()
    : QLayout(*new QStackedLayoutPrivate, 0, 0)
{
}

/*!
    Constructs a new QStackedLayout with the given \a parent.

    This layout will install itself on the \a parent widget and
    manage the geometry of its children.
*/
QStackedLayout::QStackedLayout(QWidget *parent)
    : QLayout(*new QStackedLayoutPrivate, 0, parent)
{
}

/*!
    Constructs a new QStackedLayout and inserts it into
    the given \a parentLayout.
*/
QStackedLayout::QStackedLayout(QLayout *parentLayout)
    : QLayout(*new QStackedLayoutPrivate, parentLayout, 0)
{
}

/*!
    Destroys this QStackedLayout. Note that the layout's widgets are
    \e not destroyed.
*/
QStackedLayout::~QStackedLayout()
{
    Q_D(QStackedLayout);
    qDeleteAll(d->list);
}

/*!
    Adds the given \a widget to the end of this layout and returns the
    index position of the \a widget.

    If the QStackedLayout is empty before this function is called,
    the given \a widget becomes the current widget.

    \sa insertWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedLayout::addWidget(QWidget *widget)
{
    Q_D(QStackedLayout);
    return insertWidget(d->list.count(), widget);
}

/*!
    Inserts the given \a widget at the given \a index in this
    QStackedLayout. If \a index is out of range, the widget is
    appended (in which case it is the actual index of the \a widget
    that is returned).

    If the QStackedLayout is empty before this function is called, the
    given \a widget becomes the current widget.

    Inserting a new widget at an index less than or equal to the current index
    will increment the current index, but keep the current widget.

    \sa addWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedLayout::insertWidget(int index, QWidget *widget)
{
    Q_D(QStackedLayout);
    addChildWidget(widget);
    index = qMin(index, d->list.count());
    if (index < 0)
        index = d->list.count();
    QWidgetItem *wi = new QWidgetItem(widget);
    d->list.insert(index, wi);
    invalidate();
    if (d->index < 0) {
        setCurrentIndex(index);
    } else {
        if (index <= d->index)
            ++d->index;
        widget->hide();
        widget->lower();
    }
    return index;
}

/*!
    \reimp
*/
QLayoutItem *QStackedLayout::itemAt(int index) const
{
    Q_D(const QStackedLayout);
    return d->list.value(index);
}

// Code that enables proper handling of the case that takeAt() is
// called somewhere inside QObject destructor (can't call hide()
// on the object then)

class QtFriendlyLayoutWidget : public QWidget
{
public:
    inline bool wasDeleted() const { return d_ptr->wasDeleted; }
};

static bool qt_wasDeleted(const QWidget *w) { return static_cast<const QtFriendlyLayoutWidget*>(w)->wasDeleted(); }


/*!
    \reimp
*/
QLayoutItem *QStackedLayout::takeAt(int index)
{
    Q_D(QStackedLayout);
    if (index <0 || index >= d->list.size())
        return 0;
    QLayoutItem *item = d->list.takeAt(index);
    if (index == d->index) {
        d->index = -1;
        if ( d->list.count() > 0 ) {
            int newIndex = (index == d->list.count()) ? index-1 : index;
            setCurrentIndex(newIndex);
        }
    } else if (index < d->index) {
        --d->index;
    }
    emit widgetRemoved(index);
    if (item->widget() && !qt_wasDeleted(item->widget()))
        item->widget()->hide();
    return item;
}

/*!
    \property QStackedLayout::currentIndex
    \brief the index position of the widget that is visible

    The current index is -1 if there is no current widget.

    \sa currentWidget(), indexOf()
*/
void QStackedLayout::setCurrentIndex(int index)
{
    Q_D(QStackedLayout);
    QWidget *prev = currentWidget();
    QWidget *next = widget(index);
    if (!next || next == prev)
        return;

    bool reenableUpdates = false;
    QWidget *parent = parentWidget();

    if (parent && parent->updatesEnabled()) {
        reenableUpdates = true;
        parent->setUpdatesEnabled(false);
    }

    d->index = index;
    next->raise();
    next->show();

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.

    if (parent) {
        QWidget * fw = parent->window()->focusWidget();
        if (fw && (prev && prev->isAncestorOf(fw))) { // focus was on old page
            // look for the best focus widget we can find
            if (QWidget *nfw = next->focusWidget())
                nfw->setFocus();
            else {
                // second best: first child widget in the focus chain
                QWidget *i = fw;
                while ((i = i->nextInFocusChain()) != fw) {
                    if (((i->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
                        && !i->focusProxy() && i->isVisibleTo(next) && i->isEnabled()
                        && next->isAncestorOf(i)) {
                        i->setFocus();
                        break;
                    }
                }
                // third best: incoming widget
                if (i == fw )
                    next->setFocus();
            }
        }
    }
    if (prev)
        prev->hide();
    if (reenableUpdates)
        parent->setUpdatesEnabled(true);
    emit currentChanged(index);
}

int QStackedLayout::currentIndex() const
{
    Q_D(const QStackedLayout);
    return d->index;
}


/*!
    \fn void QStackedLayout::setCurrentWidget(QWidget *widget)

    Sets the current widget to be the specified \a widget. The new
    current widget must already be contained in this stacked layout.

    \sa setCurrentIndex(), currentWidget()
 */
void QStackedLayout::setCurrentWidget(QWidget *widget)
{
    int index = indexOf(widget);
    if (index == -1) {
        qWarning("QStackedLayout::setCurrentWidget: Widget %p not contained in stack", widget);
        return;
    }
    setCurrentIndex(index);
}


/*!
    Returns the current widget, or 0 if there are no widgets in this
    layout.

    \sa currentIndex(), setCurrentWidget()
*/
QWidget *QStackedLayout::currentWidget() const
{
    Q_D(const QStackedLayout);
    return d->index >= 0 ? d->list.at(d->index)->widget() : 0;
}

/*!
    Returns the widget at the given \a index, or 0 if there is no
    widget at the given position.

    \sa currentWidget(), indexOf()
*/
QWidget *QStackedLayout::widget(int index) const
{
    Q_D(const QStackedLayout);
     if (index < 0 || index >= d->list.size())
        return 0;
    return d->list.at(index)->widget();
}

/*!
    \property QStackedLayout::count
    \brief the number of widgets contained in the layout

    \sa currentIndex(), widget()
*/
int QStackedLayout::count() const
{
    Q_D(const QStackedLayout);
    return d->list.size();
}


/*!
    \reimp
*/
void QStackedLayout::addItem(QLayoutItem *item)
{
    QWidget *widget = item->widget();
    if (widget) {
        addWidget(widget);
        delete item;
    } else {
        qWarning("QStackedLayout::addItem: Only widgets can be added");
    }
}

/*!
    \reimp
*/
QSize QStackedLayout::sizeHint() const
{
    Q_D(const QStackedLayout);
    QSize s(0, 0);
    int n = d->list.count();

    for (int i = 0; i < n; ++i)
        if (QWidget *widget = d->list.at(i)->widget())
            s = s.expandedTo(widget->sizeHint());
    return s;
}

/*!
    \reimp
*/
QSize QStackedLayout::minimumSize() const
{
    Q_D(const QStackedLayout);
    QSize s(0, 0);
    int n = d->list.count();

    for (int i = 0; i < n; ++i)
        if (QWidget *widget = d->list.at(i)->widget())
            s = s.expandedTo(qSmartMinSize(widget));
    return s;
}

/*!
    \reimp
*/
void QStackedLayout::setGeometry(const QRect &rect)
{
    QWidget *widget = currentWidget();
    if (widget)
        widget->setGeometry(rect);
}
