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

#include "qstackedwidget.h"

#include <qstackedlayout.h>
#include <qevent.h>
#include <private/qframe_p.h>

#define d d_func()
#define q q_func()

class QStackedWidgetPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QStackedWidget)
public:
    QStackedWidgetPrivate():layout(0), blockChildAdd(false){}
    QStackedLayout *layout;
    bool blockChildAdd;
};

/*!
    \class QStackedWidget qstackedwidget.h
    \brief The QStackedWidget class provides a stack of widgets where only the top
    widget is visible.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A stacked box provides a container for a number of child widgets, keeping
    all of these hidden except the widget at the top of the stack. It can be
    used to create a user interface similar to the one provided by QTabWidget.

    A stacked box can be constructed and populated with a number of child
    widgets, each of which is typically created without a parent widget:

    \code
      QStackedWidget *pages = new QStackedWidget(this);
      pages->addWidget(firstPageWidget);
      pages->addWidget(secondPageWidget);
      pages->addWidget(thirdPageWidget);
    \endcode

    The top widget in the stack is the currentWidget(). It can be changed by
    setting the \l currentIndex property, using setCurrentIndex(). The index
    of a given widget inside the stacked box is retrieved with indexOf().
    The widget() function returns the widget at a given index position.

    It is often useful to be able to control the current widget from other
    widgets. For example, a combobox could be used to hold the titles of the
    widgets in a stacked box, and used to change the current widget when an
    item is selected. This would be achieved by connecting its activated()
    signal to the setCurrent() slot of the stacked box:

    \code
      QComboBox *pageComboBox = new QComboBox(this);
      pageComboBox->insertItem("Page 1");
      pageComboBox->insertItem("Page 2");
      pageComboBox->insertItem("Page 3");
      connect(pageComboBox, SIGNAL(activated(int)),
              pages, SLOT(setCurrentIndex(int)));
    \endcode

    If you just need a stacked layout (not a widget), use QStackedLayout instead.

    \sa QTabWidget
*/

/*!
    \fn void QStackedWidget::currentChanged(int index)

    This signal is emitted when the current widget is changed. The
    parameter holds the \a index of the new current widget, or -1 if
    there isn't a new one (for example, if there are no widgets in the
    stacked box).
*/

/*!
    \fn void QStackedWidget::widgetRemoved(int index)

    This signal is emitted when the widget at position \a index is
    removed.
*/


/*!
  Constructs a new QStackedWidget as a child of \a parent.
*/

QStackedWidget::QStackedWidget(QWidget *parent)
    :QFrame(*new QStackedWidgetPrivate, parent)
{
    d->layout = new QStackedLayout(this);
    connect(d->layout, SIGNAL(widgetRemoved(int)), this, SIGNAL(widgetRemoved(int)));
}

/*!
  Destroys the object and frees any allocated resources.
*/

QStackedWidget::~QStackedWidget()
{
}

/*!  Adds \a w to this box. The first widget added becomes the
  initial current widget.  Returns the index of \a w in this box.
*/
int QStackedWidget::addWidget(QWidget *w)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->addWidget(w);
}

/*!  Inserts \a w to this box at position \a index. If \a index is out
  of range, the widget gets appened. The first widget added becomes
  the initial current widget.  Returns the index of \a w in this box.
*/
int QStackedWidget::insertWidget(int index, QWidget *w)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->insertWidget(index, w);
}


/*!
    Removes widget \a w from this layout, but does not delete it.
*/
void QStackedWidget::removeWidget(QWidget *w)
{
    d->layout->removeWidget(w);
}


/*!
    \property QStackedWidget::currentIndex
    \brief The index position of the current widget

    The current index is -1 if there is no current widget. The widget
    at index position 0 is the one that is on top (i.e. the one that
    is visible).

    \sa currentWidget() indexOf()
*/

void QStackedWidget::setCurrentIndex(int index)
{
    bool updatesEnabled = this->updatesEnabled();
    setUpdatesEnabled(false);
    d->layout->setCurrentIndex(index);
    setUpdatesEnabled(updatesEnabled);
    emit currentChanged(index);
}

int QStackedWidget::currentIndex() const
{
    return d->layout->currentIndex();
}

/*!
  Returns the current widget, or 0 if there are no child widgets.
*/
QWidget *QStackedWidget::currentWidget() const
{
    return d->layout->currentWidget();
}


/*!
  Returns the index of \a w, or -1 if \a w is not a child.
 */
int QStackedWidget::indexOf(QWidget *w) const
{
    return d->layout->indexOf(w);
}

/*!
    Returns the widget at position \a index, or 0 if there is no such
    widget.
*/
QWidget *QStackedWidget::widget(int index) const
{
    return d->layout->widget(index);
}

/*!
    \property QStackedWidget::count
    \brief The number of widgets in this layout.
*/
int QStackedWidget::count() const
{
    return d->layout->count();
}

/*!
  \reimp
*/
void QStackedWidget::childEvent(QChildEvent *e)
{
    if (!e->child()->isWidgetType())
        return;
    QWidget *w = static_cast<QWidget*>(e->child());

    if (e->added() && !d->blockChildAdd && !w->isWindow() && d->layout->indexOf(w) < 0)
        d->layout->addWidget(w);
}
