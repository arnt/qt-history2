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
    \class QStackedWidget
    \brief The QStackedWidget class provides a stack of widgets where
    only one widget is visible at a time.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QStackedWidget can be used to create a user interface similar to
    the one provided by QTabWidget. It is a convenience layout widget
    built on top of QStackedLayout.

    A QStackedWidget can be constructed and populated with a number
    of child widgets ("pages"), each of which can be created without
    a parent widget:

    \code
        QWidget *firstPageWidget = new QWidget;
        QWidget *secondPageWidget = new QWidget;
        QWidget *thirdPageWidget = new QWidget;
        ...

        QStackedWidget *stackedWidget = new QStackedWidget(this);
        stackedWidget->addWidget(firstPageWidget);
        stackedWidget->addWidget(secondPageWidget);
        stackedWidget->addWidget(thirdPageWidget);
    \endcode

    When inserted, the widgets are added to an internal list. The
    indexOf() function returns the index of a widget in that list.
    The widget() function returns the widget at a given index
    position. The widget that is shown on screen is the currentWidget().
    It can be changed using setCurrentIndex().

    QStackedWidget provides no intrinsic means for the user to switch
    page. This is typically done through a QComboBox or a QListWidget
    that stores the titles of the QStackedWidget's pages. For
    example:

    \code
        QComboBox *pageComboBox = new QComboBox(this);
        pageComboBox->addItem(tr("Page 1"));
        pageComboBox->addItem(tr("Page 2"));
        pageComboBox->addItem(tr("Page 3"));
        connect(pageComboBox, SIGNAL(activated(int)),
                stackedWidget, SLOT(setCurrentIndex(int)));
    \endcode

    \sa QStackedLayout, QTabWidget
*/

/*!
    \fn void QStackedWidget::currentChanged(int index)

    This signal is emitted when the current widget is changed. The
    parameter holds the \a index of the new current widget, or -1 if
    there isn't a new one (for example, if there are no widgets in
    the QStackedWidget).

    \sa widgetRemoved(), indexOf()
*/

/*!
    \fn void QStackedWidget::widgetRemoved(int index)

    This signal is emitted when the widget at position \a index is
    removed.

    \sa currentChanged()
*/

/*!
    Constructs a QStackedWidget with the given \a parent.
*/
QStackedWidget::QStackedWidget(QWidget *parent)
    : QFrame(*new QStackedWidgetPrivate, parent)
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

/*!
    Adds \a widget to this QStackedWidget and returns the index
    position of \a widget.

    If the QStackedWidget is empty before this function is called,
    \a widget becomes the current widget.

    \sa insertWidget(), removeWidget(), currentWidget()
*/
int QStackedWidget::addWidget(QWidget *widget)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->addWidget(widget);
}

/*!
    Inserts \a widget at position \a index in this QStackedWidget. If
    \a index is out of range, the widget is appended. Returns the
    actual index of \a widget.
    
    If the QStackedWidget was empty before this function is called,
    \a widget becomes the current widget.

    \sa addWidget(), removeWidget(), count(), currentWidget()
*/
int QStackedWidget::insertWidget(int index, QWidget *widget)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->insertWidget(index, widget);
}

/*!
    Removes \a widget from the QStackedWidget's layout. The widget is
    \e not deleted.

    \sa addWidget(), insertWidget(), currentWidget()
*/
void QStackedWidget::removeWidget(QWidget *widget)
{
    d->layout->removeWidget(widget);
}

/*!
    \property QStackedWidget::currentIndex
    \brief the index position of the widget that is visible

    The current index is -1 if there is no current widget.

    \sa currentWidget(), indexOf()
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

    Equivalent to widget(currentIndex()).

    \sa currentIndex()
*/
QWidget *QStackedWidget::currentWidget() const
{
    return d->layout->currentWidget();
}

/*!
    Returns the index of \a widget, or -1 if \a widget is not a child
    of QStackedWidget.

    \sa currentIndex(), widget()
*/
int QStackedWidget::indexOf(QWidget *widget) const
{
    return d->layout->indexOf(widget);
}

/*!
    Returns the widget at position \a index, or 0 if there is no such
    widget.

    \sa indexOf()
*/
QWidget *QStackedWidget::widget(int index) const
{
    return d->layout->widget(index);
}

/*!
    \property QStackedWidget::count
    \brief the number of child widgets

    \sa currentIndex(), widget()
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
    QWidget *widget = static_cast<QWidget*>(e->child());

    if (e->added() && !d->blockChildAdd && !widget->isWindow() && d->layout->indexOf(widget) < 0)
        d->layout->addWidget(widget);
}
