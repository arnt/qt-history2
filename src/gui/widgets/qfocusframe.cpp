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
    void updateMask();
    void update();
    QStyleOption getStyleOption() const;
};

void QFocusFramePrivate::update()
{
    Q_Q(QFocusFrame);
    q->setParent(widget->parentWidget());
    updateSize();
    if (q->parentWidget()->rect().contains(q->geometry())) {
        q->stackUnder(widget);
        q->show();
    }
}

void QFocusFramePrivate::updateSize()
{
    Q_Q(QFocusFrame);
    int vmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameVMargin),
        hmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    QRect geom(widget->x()-hmargin, widget->y()-vmargin,
               widget->width()+(hmargin*2), widget->height()+(vmargin*2));
    q->setGeometry(geom);
}

void QFocusFrame::updateMask()
{
    Q_D(QFocusFrame);
    if (!d->widget)
        return;
    QStyleHintReturnMask mask;
    QStyleOption opt = d->getStyleOption();
    if (style()->styleHint(QStyle::SH_FocusFrame_Mask, &opt, this, &mask))
        setMask(mask.region);
}

QStyleOption QFocusFramePrivate::getStyleOption() const
{
    Q_Q(const QFocusFrame);
    QStyleOption opt;
    opt.init(widget);
    opt.rect = q->rect();
    return opt;
}


QFocusFrame::QFocusFrame(QWidget *widget)
    : QWidget(*new QFocusFramePrivate, widget ? widget->parentWidget() : 0, 0)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAutoMask(true);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_NoSystemBackground, true);
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
    if(widget && !widget->isTopLevel()) {
        d->widget = widget;
        widget->installEventFilter(this);
        d->update();
    } else {
        d->widget = 0;
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
    if(o == d->widget) {
        switch(e->type()) {
        case QEvent::Move:
        case QEvent::Resize:
            d->updateSize();
            break;
        case QEvent::Hide:
            hide();
            break;
        case QEvent::Show:
        case QEvent::ParentChange:
            d->update();
            break;
        case QEvent::PaletteChange:
            setPalette(d->widget->palette());
            break;
        case QEvent::ZOrderChange:
            stackUnder(d->widget);
            break;
        case QEvent::Destroy:
            setWidget(0);
            break;
        default:
            break;
        }
    }
    return false;
}
