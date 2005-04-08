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

#include "qscrollarea.h"
#include "qscrollbar.h"
#include "private/qabstractscrollarea_p.h"
#include "qlayout.h"
#include "private/qlayoutengine_p.h"
class QScrollAreaPrivate: public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QScrollArea)

public:
    QScrollAreaPrivate(): resizable(false){}
    void updateScrollBars();
    QPointer<QWidget> widget;
    mutable QSize widgetSize;
    bool resizable;
};

/*!
    \class QScrollArea
    \brief The QScrollArea class provides a scrolling view onto another widget.

    \ingroup basic
    \mainclass

    A scroll area is used to display the contents of a child widget within
    a frame. If the widget exceeds the size of the frame, the view can
    provide scroll bars so that the entire area of the child widget can be
    viewed.

    The child widget must be specified with setWidget(); it can be
    retrieved with widget(). The view can be made to be resizable with
    the setWidgetResizable() function.

    When using a scroll area to display the contents of a custom widget,
    it is important to ensure that the \l{QWidget::sizeHint}{size hint} of the
    child widget is set to a suitable value. If a standard QWidget is used
    for the child widget, it may be necessary to call
    \l{QWidget::setMinimumSize} to ensure that the contents of the widget are
    shown correctly within the scroll area.
*/


/*!
    Constructs a scroll area with the given \a parent, and with no
    widget; see setWidget().
*/
QScrollArea::QScrollArea(QWidget *parent)
    :QAbstractScrollArea(*new QScrollAreaPrivate,parent)
{
    Q_D(QScrollArea);
    d->viewport->setBackgroundRole(QPalette::NoRole);
    d->vbar->setSingleStep(20);
    d->hbar->setSingleStep(20);
    d->layoutChildren();
}

/*!
    Destroys the scroll area.
*/
QScrollArea::~QScrollArea()
{
}

void QScrollAreaPrivate::updateScrollBars()
{
    Q_Q(QScrollArea);
    if (!widget)
        return;
    QSize p = viewport->size();
    QSize m = q->maximumViewportSize();

    QSize min = qSmartMinSize(widget);
    QSize max = qSmartMaxSize(widget);
    if ((resizable && m.expandedTo(min) == m && m.boundedTo(max) == m)
        || (!resizable && m.expandedTo(widget->size()) == m))
        p = m; // no scroll bars needed

    if (resizable)
        widget->resize(p.expandedTo(min).boundedTo(max));
    QSize v = widget->size();

    hbar->setRange(0, v.width() - p.width());
    hbar->setPageStep(p.width());
    vbar->setRange(0, v.height() - p.height());
    vbar->setPageStep(p.height());
}

/*!
    Returns the view widget's widget, or 0 if there is none.

    \sa setWidget()
*/

QWidget *QScrollArea::widget() const
{
    Q_D(const QScrollArea);
    return d->widget;
}

/*!
    Set's the view widget's widget to \a w.

    \a w becomes a child of the scroll area, and will be destroyed
    when the scroll area is deleted or when a new view widget is set.

    \sa widget()
*/
void QScrollArea::setWidget(QWidget *w)
{
    Q_D(QScrollArea);
    if (w == d->widget || !w)
        return;

    delete d->widget;
    d->widget = 0;
    d->hbar->setValue(0);
    d->vbar->setValue(0);
    if (w->parentWidget() != d->viewport)
        w->setParent(d->viewport);
    else
        w->move(0,0);
     if (!w->testAttribute(Qt::WA_Resized))
         w->resize(w->sizeHint());
    d->widget = w;
    w->installEventFilter(this);
    d->widgetSize = QSize();
    d->updateScrollBars();

}

/*!  Removes the view widget's widget from the view, and passes
  ownership of the widget to the caller.
 */
QWidget *QScrollArea::takeWidget()
{
    Q_D(QScrollArea);
    QWidget *w = d->widget;
    d->widget = 0;
    if (w)
        w->setParent(0);
    return w;
}

/*!\reimp
 */
bool QScrollArea::event(QEvent *e)
{
    Q_D(QScrollArea);
    if (e->type() == QEvent::StyleChange) {
        d->updateScrollBars();
    }
    return QAbstractScrollArea::event(e);
}


/*!\reimp
 */
bool QScrollArea::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QScrollArea);
    if (o == d->widget && e->type() == QEvent::Resize)
        d->updateScrollBars();
    return false;
}

/*! \reimp
 */
void QScrollArea::resizeEvent(QResizeEvent *)
{
    Q_D(QScrollArea);
    d->updateScrollBars();
}


/*!\reimp
 */
void QScrollArea::scrollContentsBy(int, int)
{
    Q_D(QScrollArea);
    if (!d->widget)
        return;
    d->widget->move(-d->hbar->value(), -d->vbar->value());
}


/*!
    \property QScrollArea::widgetResizable
    \brief whether the scroll area should resize the view widget

    If this property is set to false (the default), the view honors
    the size of its widget. Regardless of this property, you can
    programmatically resize the widget using widget()->resize(), and
    the scroll area will automatically adjust itself to the new size.

    If this property is set to true, the view will automatically
    resize the widget in order to avoid scroll bars where they can be
    avoided, or to take advantage of extra space.
*/
bool QScrollArea::widgetResizable() const
{
    Q_D(const QScrollArea);
    return d->resizable;
}

void QScrollArea::setWidgetResizable(bool resizable)
{
    Q_D(QScrollArea);
    d->resizable = resizable;
    updateGeometry();
    d->updateScrollBars();
}

/*!\reimp
 */
QSize QScrollArea::sizeHint() const
{
    Q_D(const QScrollArea);
    int f = 2 * d->frameWidth;
    QSize sz(f, f);
    int h = fontMetrics().height();
    if (d->widget) {
        if (!d->widgetSize.isValid())
            d->widgetSize = d->resizable ? d->widget->sizeHint() : d->widget->size();
        sz += d->widgetSize;
    } else {
        sz += QSize(12 * h, 8 * h);
    }
    if (d->vbarpolicy == Qt::ScrollBarAlwaysOn)
        sz.setWidth(sz.width() + d->vbar->sizeHint().width());
    if (d->hbarpolicy == Qt::ScrollBarAlwaysOn)
        sz.setHeight(sz.height() + d->hbar->sizeHint().height());
    return sz.boundedTo(QSize(36 * h, 24 * h));
}



/*!\reimp
 */
bool QScrollArea::focusNextPrevChild(bool next)
{
    Q_D(QScrollArea);
    if (QWidget::focusNextPrevChild(next)) {
        if (QWidget *fw = focusWidget()) {
            if (d->widget && fw != d->widget && d->widget->isAncestorOf(fw)) {
                QRect focusRect(fw->mapTo(d->widget, QPoint(0,0)), fw->size());
                QRect visibleRect(-d->widget->pos(), d->viewport->size());
                if (!visibleRect.contains(focusRect)) {
                    if (focusRect.right() > visibleRect.right())
                        d->hbar->setValue(focusRect.right() - d->viewport->width());
                    else if (focusRect.left() < visibleRect.left())
                        d->hbar->setValue(focusRect.left());
                    if (focusRect.bottom() > visibleRect.bottom())
                        d->vbar->setValue(focusRect.bottom() - d->viewport->height());
                    else if (focusRect.top() < visibleRect.top())
                        d->vbar->setValue(focusRect.top());
                }
            }
        }
        return true;
    }
    return false;
}
