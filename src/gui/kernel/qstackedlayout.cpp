#include "qstackedlayout.h"

#include <qlist.h>
#include <qwidget.h>

class QStackedLayoutPrivate
{
public:
    QStackedLayoutPrivate():index(-1){}
    QList<QLayoutItem*> list;
    int index;
};

/*!
  \class QStackedLayout

  \brief The  QStackedLayout class maintains a stack of widgets where only the top widget is visible.

    \ingroup geomanagement
    \ingroup appearance

    QStackedLayout places all its maintained widgets on top of each
    other. Only one widget is visible to the user at a time. This top
    widget is the currentWidget().  It can be changed by setting the
    \l currentIndex property, using setCurrentIndex(). The index of a
    given widget inside the stacked box is retrieved with index(),
    widget() returns the widget at a given index position.  :
    currentWidget(). The current widget is set with
    setCurrentWidget().


    \sa QStackedBox
*/

/*!
  Constructs a new QStackedLayout with parent widget \a parent.
*/

QStackedLayout::QStackedLayout(QWidget *parent)
    :QLayout(parent)
{
    d = new QStackedLayoutPrivate;
}


/*!
  Constructs a new QStackedLayout and inserts it into
  \a parentLayout.
*/

QStackedLayout::QStackedLayout(QLayout *parentLayout)
    :QLayout(parentLayout)
{
    d = new QStackedLayoutPrivate;
}

/*!
    Destroys this QStackedLayout.

    The layout's widgets aren't destroyed.
*/
QStackedLayout::~QStackedLayout()
{
    delete d;
}

/*!  Adds \a w to this layout. The first widget added becomes the
  initial current widget.  Returns the index of \a w in this layout.
*/
int QStackedLayout::addWidget(QWidget *w)
{
    return insertWidget(d->list.count(), w);
}

/*!  Inserts \a w to this layout at position \a index. If \a index is
  out of range, the widget gets appended. The first widget added
  becomes the initial current widget.  Returns the index of \a w in
  this layout.
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

/*\property QStackedBox::currentIndex
\brief The index of the current widget

The current index is -1 if there is no current widget.

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
                if (((i->focusPolicy() & TabFocus) == TabFocus)
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
  Returns the current widget, or 0 if there are no widgets in this layout. Equivalent to \c widget(currentIndex())
 */
QWidget *QStackedLayout::currentWidget() const
{
    return d->index >= 0 ? d->list.at(d->index)->widget() : 0;
}

/*!
  Returns the index of \a w in this layout, or -1 if \a w is not present in this layout.
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
  Returns the widget with index \a index in this layout. Returns 0 if there is no such widget.
 */
QWidget *QStackedLayout::widget(int index) const
{
     if (index < 0 || index >= d->list.size())
        return 0;
    return d->list.at(index)->widget();
}

/*!\property QStackedLayout::count
  \brief the number of widgets in this layout.
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
