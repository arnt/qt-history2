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

#define d d_func()
#define q q_func()

class QStackedLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QStackedLayout)
public:
    QStackedLayoutPrivate():index(-1){}
    QList<QLayoutItem*> list;
    int index;
};

/*!
    \class QStackedLayout

    \brief The QStackedLayout class maintains a stack of widgets where only the top widget is visible.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QStackedLayout places all the widgets it is responsible for on top of each
    other, so only the top one is visible to the user at any one time.
    Widget positions are given by positive integer indexes starting at
    0; the widget on top has index position 0.

    Widgets are added at the the end of the stack with addWidget(), or
    inserted in the stack with insertWidget(). The current widget is
    returned by currentWidget(), and its index is returned by
    currentIndex(). The index position of a given widget is returned
    by indexOf(), and the widget at a given index position by
    widget(). The number of widgets in the stack is returned by
    count(). The top widget is set with setCurrentIndex(). If a widget
    is removed (e.g. with QLayout::removeWidget()), the
    widgetRemoved() signal is emitted.

    \sa QStackedWidget
*/

/*!
    \property StackedLayout::count
    \brief the number of widgets contained in the layout
*/

/*!
    \fn void QStackedLayout::widgetRemoved(int index)

    This signal is emitted if a widget is removed from the stack, e.g.
    with QLayout::removeWidget(). The position that the removed widget
    occupied in the stack is given by \a index.
*/


/*!
    Constructs a new QStackedLayout
*/
QStackedLayout::QStackedLayout()
    :QLayout(*new QStackedLayoutPrivate, 0, 0)
{
}

/*!
    Constructs a new QStackedLayout with the given \a parent.
*/

QStackedLayout::QStackedLayout(QWidget *parent)
    :QLayout(*new QStackedLayoutPrivate, 0, parent)
{
}


/*!
    Constructs a new QStackedLayout and inserts it into
    the given \a parentLayout.
*/

QStackedLayout::QStackedLayout(QLayout *parentLayout)
    :QLayout(*new QStackedLayoutPrivate, parentLayout, 0)
{
}

/*!
    Destroys this QStackedLayout.

    The layout's widgets are \e not destroyed.
*/
QStackedLayout::~QStackedLayout()
{
    qDeleteAll(d->list);
}

/*!
    Adds widget \a w at the end of this layout stack and returns the
    index position of \a w.

    The very first widget that is added becomes the initial current
    widget and is placed in index position 0 (i.e. on top).

    \sa insertWidget()
*/
int QStackedLayout::addWidget(QWidget *w)
{
    return insertWidget(d->list.count(), w);
}

/*!
    Inserts widget \a w to this layout at position \a index. If \a index is
    out of range, the widget is appended. Returns the index position
    of \a w.

    The very first widget that is added becomes the initial current
    widget and is placed in index position 0 (i.e. on top).

    \sa addWidget()
*/
int QStackedLayout::insertWidget(int index, QWidget *w)
{
    addChildWidget(w);
    index = qMin(index, d->list.count());
    if (index < 0)
        index = d->list.count();
    QWidgetItem *wi = new QWidgetItem(w);
    d->list.insert(index, wi);
    invalidate();
    if (d->index < 0) {
        setCurrentIndex(index);
    } else {
        w->hide();
        w->lower();
    }
    return index;
}

/*!\reimp*/
QLayoutItem *QStackedLayout::itemAt(int index) const
{
    return d->list.value(index);
}

/*!\reimp*/
QLayoutItem *QStackedLayout::takeAt(int index)
{
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
    \brief The index position of the current widget.

    The current index is -1 if there is no current widget, for example
    if the stack is empty.

    \sa currentWidget() indexOf()
*/
void QStackedLayout::setCurrentIndex(int index)
{
    QWidget *prev = currentWidget();
    QWidget *next = widget(index);
    if (!next || next == prev)
        return;
    d->index = index;
    next->raise();
    next->show();

    // try to move focus onto the incoming widget if focus
    // was somewhere on the outgoing widget.

    Q_ASSERT(parentWidget());

    QWidget * fw = parentWidget()->topLevelWidget()->focusWidget();
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

    if (prev)
        prev->hide();
}

int QStackedLayout::currentIndex() const
{
    return d->index;
}



/*!
    Returns the current widget, or 0 if there are no widgets in this
    layout. Equivalent to \c{widget(currentIndex())}.
*/
QWidget *QStackedLayout::currentWidget() const
{
    return d->index >= 0 ? d->list.at(d->index)->widget() : 0;
}

/*!
    Returns the index position of widget \a w in this layout, or -1 if
    \a w is not in this layout.
*/
int QStackedLayout::indexOf(QWidget *w) const
{
    for (int i = 0; i < d->list.count(); ++i) {
        if (d->list.at(i)->widget() == w)
            return i;
    }
    return -1;
    //return d->list.indexOf(w);
}


/*!
    Returns the widget at position \a index, or 0 if there is no
    widget at the given position.
*/
QWidget *QStackedLayout::widget(int index) const
{
     if (index < 0 || index >= d->list.size())
        return 0;
    return d->list.at(index)->widget();
}


/*!
    Returns the number of widgets in this stacked layout.
*/
int QStackedLayout::count() const
{
    return d->list.size();
}


/*!
  \reimp
*/
void QStackedLayout::addItem(QLayoutItem *item)
{
    QWidget *w = item->widget();
    if (w) {
        addWidget(w);
        delete item;
    } else {
        qWarning("QStackedLayout::addItem(), only widgets can be added");
    }
}


/*!
  \reimp
*/
QSize QStackedLayout::sizeHint() const
{
    QSize s( 0, 0 );
    int n = d->list.count();

    for (int i = 0; i < n; ++i)
        if (QWidget *w=d->list.at(i)->widget())
            s = s.expandedTo( w->sizeHint() );

    return s;
}

/*!
  \reimp
*/
QSize QStackedLayout::minimumSize() const
{
    QSize s( 0, 0 );
    int n = d->list.count();

    for (int i = 0; i < n; ++i)
        if (QWidget *w=d->list.at(i)->widget())
            s = s.expandedTo( w->minimumSizeHint() );

    return s;
}

/*!
  \reimp
*/
void QStackedLayout::setGeometry(const QRect &rect)
{
    QWidget *w = currentWidget();
    if (w)
        w->setGeometry(rect);
}
