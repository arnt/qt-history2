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

#ifndef QT_NO_SCROLLAREA

#include "qscrollbar.h"
#include "private/qabstractscrollarea_p.h"
#include "qlayout.h"
#include "qapplication.h"
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

    \brief The QScrollArea class provides a scrolling view onto
    another widget.

    \ingroup basic
    \mainclass

    A scroll area is used to display the contents of a child widget
    within a frame. If the widget exceeds the size of the frame, the
    view can provide scroll bars so that the entire area of the child
    widget can be viewed. The child widget must be specified with
    setWidget(). For example:

    \code
        QLabel imageLabel = new QLabel;
        QImage image("happyguy.png");
        imageLabel->setPixmap(QPixmap::fromImage(image));

        scrollArea = new QScrollArea;
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageLabel);
    \endcode

    The code above creates a scroll area (shown in the images below)
    containing an image label. When scaling the image, the scroll area
    can provide the necessary scroll bars:

    \table
    \row
    \o \inlineimage qscrollarea-noscrollbars.png
    \o \inlineimage qscrollarea-onescrollbar.png
    \o \inlineimage qscrollarea-twoscrollbars.png
    \endtable

    The scroll bars appearance depends on the currently set \l
    {Qt::ScrollBarPolicy}{scroll bar policies}. You can control the
    appearance of the scroll bars using the inherited functionality
    from QAbstractScrollArea.

    For example, you can set the
    QAbstractScrollArea::horizontalScrollBarPolicy and
    QAbstractScrollArea::verticalScrollBarPolicy properties. Or if you
    want the scroll bars to adjust dynamically when the contents of
    the scroll area changes, you can use the \l
    {QAbstractScrollArea::horizontalScrollBar()}{horizontalScrollBar()}
    and \l
    {QAbstractScrollArea::verticalScrollBar()}{verticalScrollBar()}
    functions (which enable you to access the scroll bars) and set the
    scroll bars' values whenever the scroll area's contents change,
    using the QScrollBar::setValue() function.

    You can retrieve the child widget usning the widget()
    function. The view can be made to be resizable with the
    setWidgetResizable() function.

    When using a scroll area to display the contents of a custom
    widget, it is important to ensure that the
    \l{QWidget::sizeHint}{size hint} of the child widget is set to a
    suitable value. If a standard QWidget is used for the child
    widget, it may be necessary to call QWidget::setMinimumSize() to
    ensure that the contents of the widget are shown correctly within
    the scroll area.

    For a complete example using the QScrollArea class, see the \l
    {widgets/imageviewer}{ImageViewer example}. The example shows how
    to combine QLabel and QScrollArea to display an image.

    \sa QAbstractScrollArea, QScrollBar
*/


/*!
    Constructs an empty scroll area with the given \a parent.

    \sa setWidget()
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
    Destroys the scroll area and its child widget.

    \sa setWidget()
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
    Returns the scroll area's widget, or 0 if there is none.

    \sa setWidget()
*/

QWidget *QScrollArea::widget() const
{
    Q_D(const QScrollArea);
    return d->widget;
}

/*!
    \fn void QScrollArea::setWidget(QWidget *widget)

    Sets the scroll area's \a widget.

    The \a widget becomes a child of the scroll area, and will be
    destroyed when the scroll area is deleted or when a new widget is
    set.

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

/*!
    Removes the scroll area's widget, and passes ownership of the
    widget to the caller.

    \sa widget()
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

/*!
    \reimp
 */
bool QScrollArea::event(QEvent *e)
{
    Q_D(QScrollArea);
    if (e->type() == QEvent::StyleChange) {
        d->updateScrollBars();
    }
#ifdef QT_KEYPAD_NAVIGATION
    else if (QApplication::keypadNavigationEnabled()) {
        if (e->type() == QEvent::Show)
            QApplication::instance()->installEventFilter(this);
        else if (e->type() == QEvent::Hide)
            QApplication::instance()->removeEventFilter(this);
    }
#endif
    return QAbstractScrollArea::event(e);
}


/*!
    \reimp
 */
bool QScrollArea::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QScrollArea);
#ifdef QT_KEYPAD_NAVIGATION
    if (d->widget && o != d->widget && e->type() == QEvent::FocusIn
            && QApplication::keypadNavigationEnabled()) {
        if (o->isWidgetType()) {
            QWidget *w = (QWidget*)o;
            if (d->widget->isAncestorOf(w)) {
                QRect focusRect(w->mapTo(d->widget, QPoint(0,0)), w->size());
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
    }
#endif
    if (o == d->widget && e->type() == QEvent::Resize) {
        d->updateScrollBars();
        d->widget->move(-d->hbar->value(), -d->vbar->value());
	}
    return false;
}

/*!
    \reimp
 */
void QScrollArea::resizeEvent(QResizeEvent *)
{
    Q_D(QScrollArea);
    d->updateScrollBars();
    if (d->widget)
        d->widget->move(-d->hbar->value(), -d->vbar->value());
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

    If this property is set to false (the default), the scroll area
    honors the size of its widget. Regardless of this property, you
    can programmatically resize the widget using widget()->resize(),
    and the scroll area will automatically adjust itself to the new
    size.

    If this property is set to true, the scroll area will
    automatically resize the widget in order to avoid scroll bars
    where they can be avoided, or to take advantage of extra space.
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

/*!
    \reimp
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



/*!
    \reimp
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

/*!
    Scrolls the content so that the point (x, y) is visible with (xmargin, ymargin) pixel margins (if possible).
    Otherwise scrolls to the nearest valid position. Default value for both margins is 50 pixels.
    
    Note: If you call this function before entering an event loop, make sure that you have set 
    the focus on the QScrollArea widget. Otherwise, the content will be scrolled back.
    
    \code
        QScrollArea sa;
        sa.setBackgroundRole(QPalette::Dark);
        sa.setWidget(childWidget);                     
        sa.show();
        
        sa.setFocus();
        sa.ensureVisible(640, 480, 10, 10);
        
        qapp.exec();
    \endcode    
*/
void QScrollArea::ensureVisible(int x, int y, int xmargin, int ymargin)
{
    Q_D(QScrollArea);   
        
    if (x < d->hbar->value() - xmargin){
        d->hbar->setValue(qMax(0, x - xmargin));
    } else if (x > d->hbar->value() + d->viewport->width() - xmargin) {
        d->hbar->setValue(qMin(x - d->viewport->width() + xmargin, d->hbar->maximum()));
    }
    
    if (y < d->vbar->value() - ymargin){
        d->vbar->setValue(qMax(0, y - ymargin));
    } else if (y > d->vbar->value() + d->viewport->height() - ymargin) {
        d->vbar->setValue(qMin(y - d->viewport->height() + ymargin, d->vbar->maximum()));
    }                   
}

#endif // QT_NO_SCROLLAREA
