/****************************************************************************
**
** Implementation of QViewport widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qviewport.h"
#include "qscrollbar.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qevent.h"

#include "qviewport_p.h"
#include <qwidget.h>
#define d d_func()
#define q q_func()

inline  bool QViewportPrivate::viewportEvent(QEvent *e) { return q->viewportEvent(e); }


class QViewportHelper : public QWidget
{
public:
    QViewportHelper(QWidget *parent):QWidget(parent){}
    bool event(QEvent *e);
    friend class QViewport;
};
bool QViewportHelper::event(QEvent *e) {
    if (QViewport* viewport = qt_cast<QViewport*>(parentWidget()))
	return ((QViewportPrivate*)((QViewportHelper*)viewport)->d_ptr)->viewportEvent(e);
    return QWidget::event(e);
}

QViewportPrivate::QViewportPrivate()
    :hbar(0), vbar(0), vbarpolicy(ScrollBarAsNeeded), hbarpolicy(ScrollBarAsNeeded),
     viewport(0), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0)
{
}


void QViewportPrivate::init()
{
    q->setFocusPolicy(QWidget::StrongFocus);
    hbar = new QScrollBar(Horizontal,  q);
    hbar->setObjectNameConst("qt_hbar");
    QObject::connect(hbar, SIGNAL(valueChanged(int)), q, SLOT(hslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), QueuedConnection);
    vbar = new QScrollBar(Vertical, q);
    vbar->setObjectNameConst("qt_vbar");
    QObject::connect(vbar, SIGNAL(valueChanged(int)), q, SLOT(vslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), QueuedConnection);
    viewport = new QViewportHelper(q);
//    viewport->setBackgroundRole(QPalette::Base);
    QApplication::sendEvent(viewport, new QEvent(QEvent::User));
    viewport->setObjectNameConst("qt_viewport");
}

void QViewportPrivate::layoutChildren()
{
    bool needh = (hbarpolicy == ScrollBarAlwaysOn
		  || (hbarpolicy == ScrollBarAsNeeded && hbar->minimum() < hbar->maximum()));

    bool needv = (vbarpolicy == ScrollBarAlwaysOn
		  || (vbarpolicy == ScrollBarAsNeeded && vbar->minimum() < vbar->maximum()));

    int hsbExt = hbar->sizeHint().height();
    int vsbExt = vbar->sizeHint().width();

    bool reverse = QApplication::reverseLayout();
    reverse = true;
    QRect vr = q->rect();
    if (q->style().styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents)) {
	QRect fr = vr;
	if (needh) {
	    fr.setBottom(fr.bottom() - hsbExt);
	    hbar->setGeometry(QStyle::visualRect(QRect(0, fr.bottom() + 1, fr.width() - (needv?vsbExt:0), hsbExt), q));
	}
	if (needv) {
	    fr.setRight(fr.right() - vsbExt);
	    vbar->setGeometry(QStyle::visualRect(QRect(fr.right() + 1, 0, vsbExt, fr.height()), q));
	}
	q->setFrameRect(QStyle::visualRect(fr, q));
	vr = q->contentsRect();
    } else {
	q->setFrameRect(vr);
	vr = q->contentsRect();
	if (needh) {
	    vr.setBottom(vr.bottom() - hsbExt);
	    hbar->setGeometry(QStyle::visualRect(QRect(vr.left(), vr.bottom() + 1, vr.width() - (needv?vsbExt:0), hsbExt), q));
	}
	if (needv) {
	    vr.setRight(vr.right() - vsbExt);
	    vbar->setGeometry(QStyle::visualRect(QRect(vr.right() + 1, vr.top(), vsbExt, vr.height()), q));
	}
	vr = QStyle::visualRect(vr, q);
    }
    hbar->setShown(needh);
    vbar->setShown(needv);
    vr.addCoords(left, top, -right, -bottom);
    viewport->setGeometry(vr); // resize the viewport last
}

QViewport::QViewport(QViewportPrivate &dd, QWidget *parent)
    :QFrame(dd, parent)
{
    d->init();
}

QViewport::QViewport(QWidget *parent)
    :QFrame(*new QViewportPrivate, parent)
{
    d->init();
}


QViewport::~QViewport()
{
}

QWidget *QViewport::viewport() const
{
    return d->viewport;
}



Qt::ScrollBarPolicy QViewport::verticalScrollBarPolicy() const
{
    return d->vbarpolicy;
}

void QViewport::setVerticalScrollBarPolicy(ScrollBarPolicy policy)
{
    d->vbarpolicy = policy;
    if (isVisible())
	d->layoutChildren();
}

QScrollBar *QViewport::verticalScrollBar() const
{
    return d->vbar;
}

Qt::ScrollBarPolicy QViewport::horizontalScrollBarPolicy() const
{
    return d->hbarpolicy;
}

void QViewport::setHorizontalScrollBarPolicy(ScrollBarPolicy policy)
{
    d->hbarpolicy = policy;
    if (isVisible())
	d->layoutChildren();
}

QScrollBar *QViewport::horizontalScrollBar() const
{
    return d->hbar;
}

void QViewport::setViewportMargins(int left, int top, int right, int bottom)
{
    d->left = left;
    d->top = top;
    d->right = right;
    d->bottom = bottom;
    d->layoutChildren();
}

bool QViewport::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
            d->layoutChildren();
            break;
    case QEvent::Paint:
        QFrame::paintEvent((QPaintEvent*)e);
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
    case QEvent::ContextMenu:
        return false;
    default:
        return QFrame::event(e);
    }
    return true;
}

bool QViewport::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
    case QEvent::Paint:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::ContextMenu:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
        return QFrame::event(e);
    case QEvent::Wheel:
        if (!QFrame::event(e)) {
	    if (static_cast<QWheelEvent*>(e)->orientation() == Horizontal)
		return QApplication::sendEvent(d->hbar, e);
	    return QApplication::sendEvent(d->vbar, e);
	}
    default:
        return d->viewport->QWidget::event(e);
    }
    return true;
}

void QViewport::resizeEvent(QResizeEvent *)
{
}

void QViewport::paintEvent(QPaintEvent*)
{
}

void QViewport::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}
void QViewport::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}
void QViewport::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}
void QViewport::mouseMoveEvent(QMouseEvent *e)
{
    e->ignore();
}
#ifndef QT_NO_WHEELEVENT
void QViewport::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
#endif

void QViewport::keyPressEvent( QKeyEvent * e)
{
    switch (e->key()) {
    case Key_PageUp:
	d->vbar->triggerAction(QScrollBar::SliderPageStepSub);
	break;
    case Key_PageDown:
	d->vbar->triggerAction(QScrollBar::SliderPageStepAdd);
	break;
    case Key_Up:
	d->vbar->triggerAction(QScrollBar::SliderSingleStepSub);
	break;
    case Key_Down:
	d->vbar->triggerAction(QScrollBar::SliderSingleStepAdd);
	break;
    default:
	e->ignore();
	return;
    }
    e->accept();
}


#ifndef QT_NO_DRAGANDDROP
void QViewport::dragEnterEvent( QDragEnterEvent * )
{
}

void QViewport::dragMoveEvent( QDragMoveEvent * )
{
}

void QViewport::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QViewport::dropEvent( QDropEvent * )
{
}


#endif

void QViewport::scrollContentsBy(int, int)
{
    viewport()->update();
}

void QViewport::hslide(int x)
{
    int dx = d->xoffset - x;
    d->xoffset = x;
    scrollContentsBy(dx, 0);
}

void QViewport::vslide(int y)
{
    int dy = d->yoffset - y;
    d->yoffset = y;
    scrollContentsBy(0, dy);
}

void QViewport::showOrHideScrollBars()
{
    d->layoutChildren();
}
