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

#include "qfocusframe.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qstylepainter.h"
#include "qstyleoption.h"
#include <private/qwidget_p.h>

class QFocusFramePrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QFocusFrame)
    QWidget *widget;

public:
    QFocusFramePrivate() {
        widget = 0;
        sendChildEvents = false;
    }
    void updateSize();
    QStyleOption getStyleOption() const;
};

void QFocusFramePrivate::updateSize()
{
    Q_Q(QFocusFrame);
    int vmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameVMargin),
        hmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    QRect geom(widget->x()-hmargin, widget->y()-vmargin,
               widget->width()+(hmargin*2), widget->height()+(vmargin*2));
    if(geom != q->geometry()) {
        bool sizeChanged = geom.size() != q->geometry().size();
        q->setGeometry(geom);
        if(sizeChanged) {
            QBitmap bm(q->size());
            bm.fill(Qt::color0);

            QStylePainter p(&bm, q);
            p.drawControlMask(QStyle::CE_FocusFrame, getStyleOption());
            p.end();

            q->setMask(bm);
        }
    }
}

QStyleOption QFocusFramePrivate::getStyleOption() const
{
    Q_Q(const QFocusFrame);
    QStyleOption opt;
    opt.init(q);
    return opt;
}


QFocusFrame::QFocusFrame(QWidget *widget)
    : QWidget(*new QFocusFramePrivate, widget ? widget->parentWidget() : 0, 0)
{
    setAttribute(Qt::WA_NoChildEventsForParent, true);
    setWidget(widget);
}


QFocusFrame::~QFocusFrame()
{
}


void
QFocusFrame::setWidget(QWidget *widget)
{
    Q_D(QFocusFrame);
    if(widget == d->widget)
        return;

    if(d->widget)
        d->widget->removeEventFilter(this);
    if(widget) {
        d->widget = widget;
        widget->installEventFilter(this);
        setParent(widget->parentWidget());
        d->updateSize();
        raise();
        show();
    } else {
        hide();
    }
}

QWidget *
QFocusFrame::widget() const
{
    Q_D(const QFocusFrame);
    return d->widget;
}

/*! \reimp */
void
QFocusFrame::paintEvent(QPaintEvent *)
{
    Q_D(QFocusFrame);
    QStylePainter p(this);
    p.drawControl(QStyle::CE_FocusFrame, d->getStyleOption());
}

/*! \reimp */
bool
QFocusFrame::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QFocusFrame);
    if(o == d->widget && (e->type() == QEvent::Move || e->type() == QEvent::Resize))
        d->updateSize();
    return false;
}
