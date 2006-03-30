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
#include "qtimer.h"

#ifndef QT_NO_RUBBERBAND

#include "qstyle.h"
#include "qstyleoption.h"
#ifdef Q_WS_MAC
#  include <private/qt_mac_p.h>
#endif

#include <qdebug.h>

#include <private/qwidget_p.h>

//We cannot use the ToolTip hint on windows since
//it gives the rubberband a shadow. It is aparently required on X11/mac
//### a rubberband window type would be a more elegant solution
#ifdef Q_WS_WIN
#define RUBBERBAND_WINDOW_TYPE Qt::FramelessWindowHint
#else
#define RUBBERBAND_WINDOW_TYPE Qt::ToolTip
#endif

class QRubberBandPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QRubberBand)
public:
    QRect rect;
    QRubberBand::Shape shape;
    QRegion clipping;
    QStyleOptionRubberBand getStyleOption() const;
    void updateMask();
};

QStyleOptionRubberBand QRubberBandPrivate::getStyleOption() const
{
    Q_Q(const QRubberBand);
    QStyleOptionRubberBand opt;
    opt.init(q);
    opt.shape = shape;
#ifndef Q_WS_MAC
    opt.opaque = true;
#else
    opt.opaque = q->windowFlags() & RUBBERBAND_WINDOW_TYPE;
#endif
    return opt;
}

/*!
    \class QRubberBand
    \brief The QRubberBand class provides a rectangle or line that can
    indicate a selection or a boundary.

    \ingroup misc
    \mainclass

    A rubber band is often used to show a new bounding area (as in a
    QSplitter or a QDockWidget that is undocking). Historically this has
    been implemented using a QPainter and XOR, but this approach
    doesn't always work properly since rendering can happen in the
    window below the rubber band, but before the rubber band has been
    "erased".

    You can create a QRubberBand whenever you need to render a rubber band
    around a given area (or to represent a single line), then call
    setGeometry(), move() or resize() to position and size it. A common
    pattern is to do this in conjunction with mouse events. For example:

    \code
        void Widget::mousePressEvent(QMouseEvent *e)
        {
            origin = e->pos(); // origin is a QPoint
            if (!rubberBand)
                rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setGeometry(QRect(origin, QSize()));
            band->show();
        }

        void Widget::mouseMoveEvent(QMouseEvent *e)
        {
            band->setGeometry(QRect(origin, e->pos()).normalized());
        }

        void Widget::mouseReleaseEvent(QMouseEvent *e)
        {
            band->hide();
            // determine selection, for example using QRect::intersects()
            // and QRect::contains().
        }
    \endcode

    If you pass a parent to QRubberBand's constructor, the rubber band will
    display only inside its parent, but stays on top of other child widgets.
    If no parent is passed, QRubberBand will act as a top-level widget.

    Call show() to make the rubber band visible; also when the
    rubber band is not a top-level. Hiding or destroying
    the widget will make the rubber band disappear. The rubber band
    can be a \l Rectangle or a \l Line (vertical or horizontal),
    depending on the shape() it was given when constructed.
*/

// ### DOC: How about some nice convenience constructors?
//QRubberBand::QRubberBand(QRubberBand::Type t, const QRect &rect, QWidget *p)
//QRubberBand::QRubberBand(QRubberBand::Type t, int x, int y, int w, int h, QWidget *p)

/*!
    Constructs a rubber band of shape \a s, with parent \a p.

    By default a rectangular rubber band (\a s is \c Rectangle) will
    use a mask, so that a small border of the rectangle is all
    that is visible. Some styles (e.g., native Mac OS X) will
    change this and call QWidget::setWindowOpacity() to make a
    semi-transparent filled selection rectangle.
*/
QRubberBand::QRubberBand(Shape s, QWidget *p)
    : QWidget(*new QRubberBandPrivate, p, (p && p->windowType() != Qt::Desktop) ? Qt::Widget : RUBBERBAND_WINDOW_TYPE)
{
    Q_D(QRubberBand);
    d->shape = s;
    setAttribute(Qt::WA_TransparentForMouseEvents);
#ifndef Q_WS_WIN
    setAttribute(Qt::WA_NoSystemBackground);
#endif //Q_WS_WIN
    setAttribute(Qt::WA_WState_ExplicitShowHide);
    setVisible(false);
#ifdef Q_WS_MAC
    if(isWindow()) {
        extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
        ChangeWindowAttributes(qt_mac_window_for(this), kWindowNoShadowAttribute, 0);
    }
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

    This enum specifies what shape a QRubberBand should have. This is
    a drawing hint that is passed down to the style system, and can be
    interpreted by each QStyle.

    \value Line A QRubberBand can represent a vertical or horizontal
                line. Geometry is still given in rect() and the line
                will fill the given geometry on most styles.

    \value Rectangle A QRubberBand can represent a rectangle. Some
                     styles will interpret this as a filled (often
                     semi-transparent) rectangle, or a rectangular
                     outline.
*/

/*!
  Returns the shape of this rubber band. The shape can only be set
  upon construction.
*/
QRubberBand::Shape QRubberBand::shape() const
{
    Q_D(const QRubberBand);
    return d->shape;
}

/*!
    \internal
*/
void QRubberBandPrivate::updateMask()
{
    Q_Q(QRubberBand);
    QStyleHintReturnMask mask;
    QStyleOptionRubberBand opt = getStyleOption();
    if (q->style()->styleHint(QStyle::SH_RubberBand_Mask, &opt, q, &mask)) {
        q->setMask(mask.region);
    } else {
        q->clearMask();
    }
}

/*!
    \reimp
*/
void QRubberBand::paintEvent(QPaintEvent *)
{
    Q_D(QRubberBand);

    QStylePainter painter(this);
    painter.drawControl(QStyle::CE_RubberBand, d->getStyleOption());
}

/*!
    \reimp
*/
void QRubberBand::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::ParentChange:
        if (parent()) {
            setWindowFlags(windowFlags() & ~RUBBERBAND_WINDOW_TYPE);
        } else {
            setWindowFlags(windowFlags() | RUBBERBAND_WINDOW_TYPE);
        }
        break;
    default:
        break;
    }

    raise();
}

/*!
    \reimp
*/
void QRubberBand::showEvent(QShowEvent *e)
{
    raise();
    e->ignore();
}

/*!
    \reimp
*/
void QRubberBand::resizeEvent(QResizeEvent *)
{
    Q_D(QRubberBand);
    d->updateMask();
}

/*!
    \reimp
*/
void QRubberBand::moveEvent(QMoveEvent *)
{
    Q_D(QRubberBand);
    d->updateMask();
}

/*!
    \fn void QRubberBand::move(const QPoint &p);

    \overload

    Moves the rubberband to point \a p.

    \sa resize()
*/

/*!
    \fn void QRubberBand::move(int x, int y);

    Moves the rubberband to point (\a x, \a y).

    \sa resize()
*/

/*!
    \fn void QRubberBand::resize(const QSize &size);

    \overload

    Resizes the rubberband so that its new size is \a size.

    \sa move()
*/

/*!
    \fn void QRubberBand::resize(int width, int height);

    Resizes the rubberband so that its width is \a width, and its
    height is \a height.

    \sa move()
*/

/*!
    \fn void QRubberBand::setGeometry(const QRect &rect)

    Sets the geometry of the rubber band to \a rect, specified in the coordinate system
    of its parent widget.

    \sa QWidget::geometry
*/
void QRubberBand::setGeometry(const QRect &geom)
{
    QWidget::setGeometry(geom);
}

/*!
    \fn void QRubberBand::setGeometry(int x, int y, int width, int height)
    \overload

    Sets the geometry of the rubberband to the rectangle whose top-left corner lies at
    the point (\a x, \a y), and with dimensions specified by \a width and \a height.
    The geometry is specified in the parent widget's coordinate system.
*/

/*! \reimp */
bool QRubberBand::event(QEvent *e)
{
    return QWidget::event(e);
}

#endif // QT_NO_RUBBERBAND
