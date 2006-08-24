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

#include "qstackedwidget.h"

#ifndef QT_NO_STACKEDWIDGET

#include <qstackedlayout.h>
#include <qevent.h>
#include <private/qframe_p.h>

class QStackedWidgetPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QStackedWidget)
public:
    QStackedWidgetPrivate():layout(0){}
    QStackedLayout *layout;
    bool blockChildAdd;
};

/*!
    \class QStackedWidget
    \brief The QStackedWidget class provides a stack of widgets where
    only one widget is visible at a time.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QStackedWidget can be used to create a user interface similar to
    the one provided by QTabWidget. It is a convenience layout widget
    built on top of the QStackedLayout class.

    Like QStackedLayout, QStackedWidget can be constructed and
    populated with a number of child widgets ("pages"):

    \quotefromfile snippets/qstackedwidget/main.cpp
    \skipto firstPageWidget
    \printto QComboBox
    \skipto QVBoxLayout
    \printline QVBoxLayout
    \skipto layout->addWidget(stackedWidget)
    \printuntil setLayout

    QStackedWidget provides no intrinsic means for the user to switch
    page. This is typically done through a QComboBox or a QListWidget
    that stores the titles of the QStackedWidget's pages. For
    example:

    \quotefromfile snippets/qstackedwidget/main.cpp
    \skipto QComboBox
    \printto QVBoxLayout

    When populating a stacked widget, the widgets are added to an
    internal list. The indexOf() function returns the index of a
    widget in that list. The widgets can either be added to the end of
    the list using the addWidget() function, or inserted at a given
    index using the insertWidget() function. The removeWidget()
    function removes the widget at the given index from the stacked
    widget. The number of widgets contained in the stacked widget, can
    be obtained using the count() function.

    The widget() function returns the widget at a given index
    position. The index of the widget that is shown on screen is given
    by currentIndex() and can be changed using setCurrentIndex(). In a
    similar manner, the currently shown widget can be retrieved using
    the currentWidget() function, and altered using the
    setCurrentWidget() function.

    Whenever the current widget in the stacked widget changes or a
    widget is removed from the stacked widget, the currentChanged()
    and widgetRemoved() signals are emitted respectively.

    \sa QStackedLayout, QTabWidget, {Config Dialog Example}
*/

/*!
    \fn void QStackedWidget::currentChanged(int index)

    This signal is emitted whenever the current widget changes.

    The parameter holds the \a index of the new current widget, or -1
    if there isn't a new one (for example, if there are no widgets in
    the QStackedWidget).

    \sa currentWidget(), setCurrentWidget()
*/

/*!
    \fn void QStackedWidget::widgetRemoved(int index)

    This signal is emitted whenever a widget is removed. The widget's
    \a index is passed as parameter.

    \sa removeWidget()
*/

/*!
    Constructs a QStackedWidget with the given \a parent.

    \sa addWidget(), insertWidget()
*/
QStackedWidget::QStackedWidget(QWidget *parent)
    : QFrame(*new QStackedWidgetPrivate, parent)
{
    Q_D(QStackedWidget);
    d->layout = new QStackedLayout(this);
    connect(d->layout, SIGNAL(widgetRemoved(int)), this, SIGNAL(widgetRemoved(int)));
    connect(d->layout, SIGNAL(currentChanged(int)), this, SIGNAL(currentChanged(int)));
}

/*!
    Destroys this stacked widget, and frees any allocated resources.
*/
QStackedWidget::~QStackedWidget()
{
}

/*!
    Appends the given \a widget to this QStackedWidget and returns the
    index position.

    If the QStackedWidget is empty before this function is called,
    \a widget becomes the current widget.

    \sa insertWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedWidget::addWidget(QWidget *widget)
{
    return d_func()->layout->addWidget(widget);
}

/*!
    Inserts the given \a widget at the given \a index in this
    QStackedWidget. If \a index is out of range, the \a widget is
    appended (in which case it is the actual index of the \a widget
    that is returned).

    If the QStackedWidget was empty before this function is called,
    the given \a widget becomes the current widget.

    Inserting a new widget at an index less than or equal to the current index
    will increment the current index, but keep the current widget.

    \sa addWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedWidget::insertWidget(int index, QWidget *widget)
{
    return d_func()->layout->insertWidget(index, widget);
}

/*!
    Removes the given \a widget from this QStackedWidget. The widget
    is \e not deleted.

    \sa addWidget(), insertWidget(), currentWidget()
*/
void QStackedWidget::removeWidget(QWidget *widget)
{
    d_func()->layout->removeWidget(widget);
}

/*!
    \property QStackedWidget::currentIndex
    \brief the index position of the widget that is visible

    The current index is -1 if there is no current widget.

    \sa currentWidget(), indexOf()
*/

void QStackedWidget::setCurrentIndex(int index)
{
    d_func()->layout->setCurrentIndex(index);
}

int QStackedWidget::currentIndex() const
{
    return d_func()->layout->currentIndex();
}

/*!
    Returns the current widget, or 0 if there are no child widgets.

    \sa currentIndex(), setCurrentWidget()
*/
QWidget *QStackedWidget::currentWidget() const
{
    return d_func()->layout->currentWidget();
}


/*!
    \fn void QStackedWidget::setCurrentWidget(QWidget *widget)

    Sets the current widget to be the specified \a widget. The new
    current widget must already be contained in this stacked widget.

    \sa currentWidget(), setCurrentIndex()
 */
void QStackedWidget::setCurrentWidget(QWidget *widget)
{
    Q_D(QStackedWidget);
    if (d->layout->indexOf(widget) == -1) {
        qWarning("QStackedWidget::setCurrentWidget: widget %p not contained in stack", widget);
        return;
    }
    d->layout->setCurrentWidget(widget);
}

/*!
    Returns the index of the given \a widget, or -1 if the given \a
    widget is not a child of this QStackedWidget.

    \sa currentIndex(), widget()
*/
int QStackedWidget::indexOf(QWidget *widget) const
{
    return d_func()->layout->indexOf(widget);
}

/*!
    Returns the widget at the given \a index, or 0 if there is no such
    widget.

    \sa currentWidget(), indexOf()
*/
QWidget *QStackedWidget::widget(int index) const
{
    return d_func()->layout->widget(index);
}

/*!
    \property QStackedWidget::count
    \brief the number of widgets contained by this stacked widget

    \sa currentIndex(), widget()
*/
int QStackedWidget::count() const
{
    return d_func()->layout->count();
}

/*! \reimp */
bool QStackedWidget::event(QEvent *e)
{
    return QFrame::event(e);
}
#endif // QT_NO_STACKEDWIDGET
