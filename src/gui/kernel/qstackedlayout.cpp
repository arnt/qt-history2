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

#include "qstackedlayout.h"
#include "qlayout_p.h"

#include <qlist.h>
#include <qwidget.h>

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
    ("pages"):

    \code
        QWidget *firstPageWidget = new QWidget(this);
        QWidget *secondPageWidget = new QWidget(this);
        QWidget *thirdPageWidget = new QWidget(this);
        ...

        QStackedLayout *layout = new QStackedLayout(this);
        layout->addWidget(firstPageWidget);
        layout->addWidget(secondPageWidget);
        layout->addWidget(thirdPageWidget);
    \endcode

    When inserted, the widgets are added to an internal list. The
    indexOf() function returns the index of a widget in that list.
    The widget() function returns the widget at a given index
    position. The index of the widget that is shown on screen is
    given by currentIndex() and can be changed using setCurrentIndex().

    QStackedLayout provides no intrinsic means for the user to switch
    page. This is typically done through a QComboBox or a QListWidget
    that stores the titles of the QStackedLayout's pages. For
    example:

    \code
        QComboBox *pageComboBox = new QComboBox(this);
        pageComboBox->addItem(tr("Page 1"));
        pageComboBox->addItem(tr("Page 2"));
        pageComboBox->addItem(tr("Page 3"));
        connect(pageComboBox, SIGNAL(activated(int)),
                layout, SLOT(setCurrentIndex(int)));
    \endcode

    \sa QStackedWidget, QTabWidget
*/

/*!
    \fn void QStackedLayout::widgetRemoved(int index)

    This signal is emitted when the widget at position \a index
    is removed from the layout.

    \sa removeWidget()
*/

/*!
    Constructs a QStackedLayout with no parent.

    This QStackedLayout must be added to another layout later on to
    become effective.
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
    Destroys this QStackedLayout.

    The layout's widgets are \e not destroyed.
*/
QStackedLayout::~QStackedLayout()
{
    Q_D(QStackedLayout);
    qDeleteAll(d->list);
}

/*!
    Adds \a widget to the end of this layout and returns the
    index position of \a widget.

    If the QStackedLayout is empty before this function is called,
    \a widget becomes the current widget.

    \sa insertWidget(), removeWidget(), currentWidget()
*/
int QStackedLayout::addWidget(QWidget *widget)
{
    Q_D(QStackedLayout);
    return insertWidget(d->list.count(), widget);
}

/*!
    Inserts \a widget at position \a index in this QStackedLayout. If
    \a index is out of range, the widget is appended. Returns the
    actual index of \a widget.

    If the QStackedLayout is empty before this function is called,
    \a widget becomes the current widget.

    \sa addWidget()
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
            int newIndex = index > 0 ? index - 1 : 0;
            setCurrentIndex(newIndex);
        }
    } else if (index < d->index) {
        --d->index;
    }
    emit widgetRemoved(index);
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

    bool updatesEnabled = false;
    QWidget *parent = parentWidget();

    if (parent) {
        updatesEnabled =  parent->updatesEnabled();
        parent->setUpdatesEnabled(false);
    }

    d->index = index;
    next->raise();
    next->show();

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.

    if (parent) {
        QWidget * fw = parent->window()->focusWidget();
        if (prev->isAncestorOf(fw)) { // focus was on old page
            // look for the best focus widget we can find
            if (QWidget *nfw = next->focusWidget())
                nfw->setFocus();
            else {
                // second best == first child widget in the focus chain
                QWidget *i = fw;
                while ((i = i->nextInFocusChain()) != fw) {
                    if (((i->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
                        && !i->focusProxy() && i->isVisibleTo(next) && i->isEnabled()
                        && next->isAncestorOf(i)) {
                        i->setFocus();
                        break;
                    }
                }
                // third best == incoming widget
                if (i == fw )
                    next->setFocus();
            }
        }
    }
    if (prev)
        prev->hide();
    if (parent)
        parent->setUpdatesEnabled(updatesEnabled);
    emit currentChanged(index);
}

int QStackedLayout::currentIndex() const
{
    Q_D(const QStackedLayout);
    return d->index;
}


/*!
   Sets \w to be the current widget. \w must be contained in this
   stacked layout.

  \sa setCurrentIndex(), currentWidget()
 */
void QStackedLayout::setCurrentWidget(QWidget *w)
{
    Q_ASSERT_X(indexOf(w) >= 0, "QStackedLayout::setCurrentWidget", "widget not contained in stack");
    setCurrentIndex(indexOf(w));
}


/*!
    Returns the current widget, or 0 if there are no widgets in this
    layout.

    Equivalent to widget(currentIndex()).

    \sa currentIndex()
*/
QWidget *QStackedLayout::currentWidget() const
{
    Q_D(const QStackedLayout);
    return d->index >= 0 ? d->list.at(d->index)->widget() : 0;
}

/*!
    Returns the widget at position \a index, or 0 if there is no
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
            s = s.expandedTo(widget->minimumSizeHint());
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
