#include "qwidgetview.h"
#include "qscrollbar.h"

#include "private/qviewport_p.h"
#include "qlayout.h"
#include "private/qlayoutengine_p.h"

class QWidgetViewPrivate: public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QWidgetView);

public:
    QWidgetViewPrivate():widget(0), resizable(false){}
    void updateScrollBars();
    QPointer<QWidget> widget;
    mutable QSize widgetSize;
    bool resizable;
};

#define d d_func()
#define q q_func()


/*!

    \class QWidgetView

    \brief The QWidgetView provides on-demand scroll bars around a widget.

    \ingroup basic
    \mainclass
 */


/*!
    Constructs a widget view.

    The \a parent arguments is sent to the QWidget constructor.
*/
QWidgetView::QWidgetView(QWidget *parent)
    :QViewport(*new QWidgetViewPrivate,parent)
{
    d->viewport->setAttribute(WA_SetBackgroundRole, false);
    d->vbar->setSingleStep(20);
    d->hbar->setSingleStep(20);
//     d->vbar->setRange(0,0);
//     d->hbar->setRange(0,0);
    d->layoutChildren();
}

/*!
  Destroys the widget view.
 */
QWidgetView::~QWidgetView()
{
}

void QWidgetViewPrivate::updateScrollBars()
{
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

    d->hbar->setRange(0, v.width() - p.width());
    d->hbar->setPageStep(p.width());
    d->vbar->setRange(0, v.height() - p.height());
    d->vbar->setPageStep(p.height());
}

/*!
  Returns the current view widget. If there is no view widget, returns 0.

  \sa setWidget()
 */

QWidget *QWidgetView::widget() const
{
    return d->widget;
}

/*!
  Makes \a w the view widget.

  \a w becomes a child of the widgetview, and will be destroyed when
  the widgetview gets deleted or a new view widget is set.

  \sa widget()
*/
void QWidgetView::setWidget(QWidget *w)
{
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
     if (!w->testAttribute(WA_Resized))
         w->resize(w->sizeHint());
    d->widget = w;
    w->installEventFilter(this);
    d->widgetSize = QSize();
    d->updateScrollBars();

}

/*!\reimp
 */
bool QWidgetView::event(QEvent *e)
{
    if (e->type() == QEvent::StyleChange) {
        d->updateScrollBars();
    }
    return QViewport::event(e);
}


/*!\reimp
 */
bool QWidgetView::eventFilter(QObject *o, QEvent *e)
{
    if (o == d->widget && e->type() == QEvent::Resize)
        d->updateScrollBars();
    return false;
}

/*! \reimp
 */
void QWidgetView::resizeEvent(QResizeEvent *)
{
    d->updateScrollBars();
}


/*!\reimp
 */
void QWidgetView::scrollContentsBy(int, int)
{
    if (!d->widget)
        return;
    d->widget->move(-d->hbar->value(), -d->vbar->value());
}


/*!
  \property QWidgetView::widgetResizable
  \brief whether the view should resize the view widget

  If this property is set to false (the default), the view keeps the
  size of the view widget. You can at anytime change resize the widget
  with widget()->resize(), and the view will adjust itself to the new
  size.

  If this property is set to true, the view automatically resizes the
  widget in order to avoid scroll bars where they can be avoided, or
  to take advantage of extra space.
 */
bool QWidgetView::widgetResizable() const
{
    return d->resizable;
}

void QWidgetView::setWidgetResizable(bool resizable)
{
    d->resizable = resizable;
    updateGeometry();
}

/*!\reimp
 */
QSize QWidgetView::sizeHint() const
{
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
    if (d->vbarpolicy == ScrollBarAlwaysOn)
        sz.setWidth(sz.width() + d->vbar->sizeHint().width());
    if (d->hbarpolicy == ScrollBarAlwaysOn)
        sz.setHeight(sz.height() + d->hbar->sizeHint().height());
    return sz.boundedTo(QSize(36 * h, 24 * h));
}



