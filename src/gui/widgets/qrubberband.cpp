/****************************************************************************
**
** Implementation of QRubberband class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qrubberband.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qbitmap.h"
#include "qevent.h"
#include "qtimer.h"

#include <private/qwidget_p.h>
class QRubberBandPrivate : public QWidgetPrivate
{
public:
    QRect rect;
    QRubberBand::Shape shape;
};
#define d d_func()
#define q q_func()

/*!
   \class QRubberBand qrubberband.h
   \brief The QRubberBand class provides a rectangle or line that can
   indicate a selection or a boundary.

    \ingroup misc
    \mainclass

    A rubber band is often used to show a new bounding area (as in a
    QSplitter or a QDockWindow that is undocking). Commonly this has
    been implemented using a QPainter and XOR, but this approach
    doesn't always work properly since rendering can happen in the
    window below the rubber band, but before the rubber band has been
    "erased".

    You can create a QRubberBand whenever you need to render a rubber
    band around a given area (or to represent a single line), then
    call setGeometry() to move and size it; hiding the widget will
    make the rubber band disappear.
*/
//### Why hide it rather than delete it?

//### How about some nice convenience constructors?
//QRubberBand::QRubberBand(QRubberBand::Type t, const QRect &rect, QWidget *p)
//QRubberBand::QRubberBand(QRubberBand::Type t, int x, int y, int w, int h, QWidget *p)

/*!
    Constructs a rubber band of shape \a s, with parent \a p.

    By default a rectangular QRubberBand (\a t is \c Rectangle) will
    be set to auto mask, so that the boundry of the rectangle is all
    that is visible. Some styles (for example native Mac OS X) will
    change this and call QWidget::setWindowOpacity() to make the
    window only partially opaque.
*/
QRubberBand::QRubberBand(QRubberBand::Shape s, QWidget *p) : 
    QWidget(*new QRubberBandPrivate, p, WType_TopLevel | WStyle_Tool | WStyle_Customize | WStyle_NoBorder)
{
    d->shape = s;
    setAutoMask(true);
}

/*!
  Destructor.
*/
QRubberBand::~QRubberBand()
{
}

/*!
    \enum QRubberBand::Shape

    \value Line
    \value Rectangle
*/

/*!
    Virtual function that draws the mask of a QRubberBand using
    painter \a p.

    The drawing is themed (using QStyle), but you can reimplement it
    to achieve custom effects.
*/
void
QRubberBand::drawRubberBandMask(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->shape == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBandMask, p, d->rect, palette(), flags);
}

/*!
    Virtual function that draws the contents of a QRubberBand using
    painter \a p.

    The drawing is themed (using QStyle), but you can reimplement it
    to achieve custom effects.
*/
void
QRubberBand::drawRubberBand(QPainter *p)
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if(d->shape == Rectangle)
	flags |= QStyle::Style_Rectangle;
    style().drawPrimitive(QStyle::PE_RubberBand, p, d->rect, palette(), flags);
}

/*!
  Returns the shape of this rubber band. The shape can only be set
  upon construction.
*/
QRubberBand::Shape 
QRubberBand::shape() const 
{ 
    return d->shape; 
}

/*!
    \reimp
*/
void 
QRubberBand::updateMask()
{
    QBitmap bm(width(), height(), true);
    QPainter p(&bm);
    drawRubberBandMask(&p);
    p.end();
    setMask(QRegion(bm));
}

/*!
    \reimp
*/
void
QRubberBand::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawRubberBand(&p);
}

/*!
    \reimp
*/
void 
QRubberBand::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange && autoMask()) 
	updateMask();
}

/*!
    \fn void QRubberBand::move(const QPoint &p);

    Moves the rubberband to point \a p.
*/

/*!
    \fn void QRubberBand::move(int x, int y);

    Moves the rubberband to point (\a x, \a y).
*/

/*!
    \fn void QRubberBand::resize(const QSize &size);

    Resizes the rubberband so that its new size is \a size.
*/

/*!
    \fn void QRubberBand::resize(int width, int height);

    Resizes the rubberband so that its width is \a width, and its
    height is \a height.
*/

/*!
    \fn void QRubberBand::setGeometry(const QRect &rect);

    Changes the rubberband's geometry to the geometry of the rectangle
    \a rect.
*/


void QRubberBand::setGeometry(int x, int y, int w, int h)
{
#if 1
    QRect mygeom(x, y, w, h);
    d->rect = QRect(0, 0, w, h);
    if(QWidget *p = parentWidget()) {
	const QRect prect(p->mapToGlobal(QPoint(0, 0)), p->size());
	if(!prect.contains(mygeom)) {
	    if(mygeom.left() < prect.left()) {
		const int diff = prect.left()-mygeom.left();
		d->rect.moveLeft(-diff);
		mygeom.moveLeft(prect.left());
		mygeom.setWidth(mygeom.width()-diff);
	    }
	    if(mygeom.top() < prect.top()) {
		const int diff = prect.top()-mygeom.top();
		d->rect.moveTop(-diff);
		mygeom.moveTop(prect.top());
		mygeom.setHeight(mygeom.height()-diff);
	    }
	    if(mygeom.left() > prect.right()) 
		mygeom.moveLeft(prect.right());
	    if(mygeom.top() > prect.bottom()) 
		mygeom.moveTop(prect.bottom());
	    if(mygeom.bottom() > prect.bottom()) {
		const int diff = mygeom.bottom()-prect.bottom();
		mygeom.setHeight(mygeom.height()-diff);
	    }
	    if(mygeom.right() > prect.right()) {
		const int diff = mygeom.right()-prect.right();
		mygeom.setWidth(mygeom.width()-diff);
	    }
	}
    }
    if(mygeom != geometry()) {
	QWidget::setGeometry(mygeom);
    } else {
	updateMask();
    }
    update();
#else
    QWidget::setGeometry(x, y, w, h);
#endif
}
