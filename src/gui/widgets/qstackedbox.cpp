#include "qstackedbox.h"

#include <qstackedlayout.h>
#include <qevent.h>
#include <private/qframe_p.h>

#define d d_func()
#define q q_func()

class QStackedBoxPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QStackedBox)
public:
    QStackedBoxPrivate();
    QStackedLayout *layout;
    bool blockAutoAdd;
};

QStackedBoxPrivate::QStackedBoxPrivate()
    :blockAutoAdd(false)
{}

/*!
    \class QStackedBox qstackedbox.h
    \brief The  QStackedBox class provides a stack of widgets where only the top widget is visible.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    All the horizontal box's child widgets will be placed on top of
    each other, and only the top one will be visible to the user. This
    top widget is the currentWidget().  It can be changed by setting
    the \l currentIndex property, using setCurrentIndex(). The index
    of a given widget inside the stacked box is retrieved with
    index(), widget() returns the widget at a given index position.
*/



/*!
  Constructs a new QStackedBox as a child of \a parent.
*/

QStackedBox::QStackedBox(QWidget *parent)
    :QFrame(*new QStackedBoxPrivate, parent)
{
    d->layout = new QStackedLayout(this);
}

/*!
  Destroys the object and frees any allocated resources.
*/

QStackedBox::~QStackedBox()
{
}

/*!  Adds \a w to this box. The first widget added becomes the
  initial current widget.  Returns the index of \a w in this box.
*/
int QStackedBox::addWidget(QWidget *w)
{
    d->blockAutoAdd = true;
    int index = d->layout->addWidget(w);
    d->blockAutoAdd = false;
    return index;
}

/*!  Inserts \a w to this box at position \a index. If \a index is
  out of range, the widget gets added. The first widget added becomes
  the initial current widget.  Returns the index of \a w in this
  box.
*/
int QStackedBox::insertWidget(int index, QWidget *w)
{
    d->blockAutoAdd = true;
    index = d->layout->insertWidget(index, w);
    d->blockAutoAdd = false;
    return index;
}

/*!
  Removes the widget at position \a index from this box.
 */
void QStackedBox::removeWidget(int index)
{
    d->layout->removeWidget(index);
}


/*\property QStackedBox::currentIndex
\brief The index of the current widget

The current index is -1 if there is no current widget.

\sa currentWidget() indexOf()
*/

void QStackedBox::setCurrentIndex(int idx)
{
    d->layout->setCurrentIndex(idx);
}

int QStackedBox::currentIndex() const
{
    return d->layout->currentIndex();
}

/*!
  Returns the current widget, or 0 if there are no child widgets.
*/
QWidget *QStackedBox::currentWidget() const
{
    return d->layout->currentWidget();
}


/*!
  Returns the index of \a w, or -1 if \a w is not a child.
 */
int QStackedBox::indexOf(QWidget *w) const
{
    return d->layout->indexOf(w);
}

/*!
  Returns the widget with index \a idx. Returns 0 if there is no such widget.
 */
QWidget *QStackedBox::widget(int index) const
{
    return d->layout->widget(index);
}

/*!\property QStackedLayout::count
  \brief the number of widgets in this layout.
*/
int QStackedBox::count() const
{
    return d->layout->count();
}

/*!
  \reimp
*/
void QStackedBox::childEvent(QChildEvent *e)
{
    if (!e->child()->isWidgetType())
        return;
    QWidget *w = static_cast<QWidget*>(e->child());

    if (e->added() && !d->blockAutoAdd
        && !w->isTopLevel() && d->layout->indexOf(w) != -1)
        d->layout->addWidget(w);
    else if (e->removed())
        d->layout->QLayout::removeWidget(w);
}
