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

#include "qbitmap.h"
#include "qevent.h"
#include "qstylepainter.h"
#include "qrubberband.h"
#include "qstyle.h"
#include "qstyleoption.h"
#ifdef Q_WS_MAC
#  include <private/qt_mac_p.h>
#endif

#include <qdebug.h>

#include <private/qwidget_p.h>
class QRubberBandPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QRubberBand)
public:
    QRect rect;
    QRubberBand::Shape shape;
    QStyleOption getStyleOption() const;
};

#define d d_func()
#define q q_func()
QStyleOption QRubberBandPrivate::getStyleOption() const
{
    QStyleOption opt(0);
    opt.rect = rect;
    opt.palette = q->palette();
    opt.state = QStyle::State_None;
    if (shape == QRubberBand::Rectangle)
        opt.state |= QStyle::State_Rectangle;
    return opt;
}

/*!
   \class QRubberBand qrubberband.h
   \brief The QRubberBand class provides a rectangle or line that can
   indicate a selection or a boundary.

    \ingroup misc
    \mainclass

    A rubber band is often used to show a new bounding area (as in a
    QSplitter or a QDockWidget that is undocking). Commonly this has
    been implemented using a QPainter and XOR, but this approach
    doesn't always work properly since rendering can happen in the
    window below the rubber band, but before the rubber band has been
    "erased".

    You can create a QRubberBand whenever you need to render a rubber
    band around a given area (or to represent a single line), then
    call setGeometry(), move() or resize() to position and size it;
    hiding (or destroying) the widget will make the rubber band
    disappear. The rubber band can be a \c Rectangle or a \c Line,
    depending on the shape() it was given when constructed.
*/

// ### DOC: How about some nice convenience constructors?
//QRubberBand::QRubberBand(QRubberBand::Type t, const QRect &rect, QWidget *p)
//QRubberBand::QRubberBand(QRubberBand::Type t, int x, int y, int w, int h, QWidget *p)

/*!
    Constructs a rubber band of shape \a s, with parent \a p.

    By default a rectangular QRubberBand (\a s is \c Rectangle) will
    be set to auto mask, so that the boundry of the rectangle is all
    that is visible. Some styles (for example native Mac OS X) will
    change this and call QWidget::setWindowOpacity() to make the
    window only partially opaque.
*/
QRubberBand::QRubberBand(Shape s, QWidget *p) :
    QWidget(*new QRubberBandPrivate, p, Qt::Overlay)
{
    d->shape = s;
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAutoMask(true);
#ifdef Q_WS_MAC
    extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
    ChangeWindowAttributes(qt_mac_window_for(this), kWindowNoShadowAttribute, 0);
#endif
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
  Returns the shape of this rubber band. The shape can only be set
  upon construction.
*/
QRubberBand::Shape QRubberBand::shape() const
{
    return d->shape;
}

/*!
    \reimp
*/
void QRubberBand::updateMask()
{
    QStyleHintReturnMask mask;
    QStyleOption opt = d->getStyleOption();
    if (style()->styleHint(QStyle::SH_RubberBand_Mask, &opt, q, &mask))
        setMask(mask.region);
}

/*!
    \reimp
*/
void QRubberBand::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.drawControl(QStyle::CE_RubberBand, d->getStyleOption());
}

/*!
    \reimp
*/
void QRubberBand::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange && autoMask())
        updateMask();
}

/*!
    \fn void QRubberBand::move(const QPoint &p);

    \overload

    Moves the rubberband to point \a p.
*/

/*!
    \fn void QRubberBand::move(int x, int y);

    Moves the rubberband to point (\a x, \a y).
*/

/*!
    \fn void QRubberBand::resize(const QSize &size);

    \overload

    Resizes the rubberband so that its new size is \a size.
*/

/*!
    \fn void QRubberBand::resize(int width, int height);

    Resizes the rubberband so that its width is \a width, and its
    height is \a height.
*/

/*!
    \fn void QRubberBand::setGeometry(int x, int y, int w, int h)

    \overload

    Changes the rubberband's geometry to have a top-left corner of (\a
    x, \a y), a width of \a w, and a height of \a h.
*/

/*!
    \fn void QRubberBand::setGeometry(const QRect &rect)

    Changes the rubberband's geometry to the geometry of the rectangle
    \a rect.
*/
void QRubberBand::setGeometry(const QRect &geom)
{
#if 1
    QRect mygeom = geom;
    d->rect = QRect(0, 0, mygeom.width(), mygeom.height());
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
    QWidget::setMygeometry(geom);
#endif
}
