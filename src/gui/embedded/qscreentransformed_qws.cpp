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


#ifndef QT_NO_QWS_TRANSFORMED

#include "qscreentransformed_qws.h"
#include <qvector.h>
#include <private/qpainter_p.h>
#include <qdebug.h>
#include <qmatrix.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <qwindowsystem_qws.h>

//#define QT_REGION_DEBUG

// Global function ---------------------------------------------------------------------------------
static QTransformedScreen *qt_trans_screen = 0;

void qws_setScreenTransformation(int t)
{
    if (qt_trans_screen) {
        qt_trans_screen->setTransformation((QTransformedScreen::Transformation)t);
        qwsServer->refresh();
    }
}

// -------------------------------------------------------------------------------------------------
// Transformed Screen
// -------------------------------------------------------------------------------------------------

/*!
    \class QTransformedScreen
    \ingroup qws

    \brief The QTransformedScreen class implements a screen driver for
    a transformed screen.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application, but there should
    only be one screen object per application.

    The default implementation of QTransformedScreen inherits the
    QVFbScreen class if the virtual framebuffer is enabled, or
    QLinuxFbScreen if it is not. But note that any QScreen subclass,
    or QScreen itself, can serve as its base class. This is easily
    achieved by manipulating the \c QT_TRANS_SCREEN_BASE definition in
    the header file.

    Use the QScreen::isTransformed() function to determine if a screen
    is transformed. The QTransformedScreen class itself provides means
    of rotating the screen with its setTransformation() function; the
    transformation() function returns the currently set rotation in
    terms of the \l Transformation enum (which describes the various
    available rotation settings). Alternatively, QTransformedScreen
    provides an implementation of the QScreen::transformOrientation()
    function, returning the current rotation as an integer value.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \enum QTransformedScreen::Transformation

    This enum describes the various rotations a transformed screen can
    have.

    \value None No rotation
    \value Rot90 90 degrees rotation
    \value Rot180 180 degrees rotation
    \value Rot270 270 degrees rotation
*/

/*!
    \fn void QTransformedScreen::blit(const QImage & img, const QPoint & topLeft, const QRegion & region)
    \reimp
*/

/*!
    \fn bool QTransformedScreen::connect(const QString & displaySpec)
    \reimp
*/

/*!
    \fn bool QTransformedScreen::isTransformed() const
    \reimp
*/

/*!
    \fn QSize QTransformedScreen::mapFromDevice(const QSize & s) const
    \reimp
*/

/*!
    \fn QPoint QTransformedScreen::mapFromDevice(const QPoint &, const QSize &) const
    \reimp
*/

/*!
    \fn QRect QTransformedScreen::mapFromDevice(const QRect &, const QSize &) const
    \reimp
*/

/*!
    \fn QRegion QTransformedScreen::mapFromDevice(const QRegion &, const QSize &) const
    \reimp
*/

/*!
    \fn QSize QTransformedScreen::mapToDevice(const QSize & s) const
    \reimp
*/

/*!
    \fn QPoint QTransformedScreen::mapToDevice(const QPoint &, const QSize &) const
    \reimp
*/

/*!
    \fn QRect QTransformedScreen::mapToDevice(const QRect &, const QSize &) const
    \reimp
*/

/*!
    \fn QRegion QTransformedScreen::mapToDevice(const QRegion &, const QSize &) const
    \reimp
*/

/*!
    \fn void QTransformedScreen::solidFill(const QColor & color, const QRegion & region)
    \reimp
*/

/*!
    \fn int QTransformedScreen::transformOrientation() const
    \reimp
*/

/*!
    \fn QTransformedScreen::QTransformedScreen(int displayId)

    Constructs a QTransformedScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/

/*!
    \fn void QTransformedScreen::setTransformation ( Transformation transformation )

    Rotates this screen object according to the specified \a transformation.

    \sa transformation()
*/

/*!
    \fn Transformation QTransformedScreen::transformation() const

    Returns the currently set rotation.

    \sa setTransformation(), QScreen::transformOrientation()
*/
QTransformedScreen::QTransformedScreen(int display_id)
    : QT_TRANS_SCREEN_BASE(display_id)
{
    trans = None;
    qt_trans_screen = this;

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::QTransformedScreen";
#endif
}

bool QTransformedScreen::connect(const QString &displaySpec)
{
    bool result = QT_TRANS_SCREEN_BASE::connect(displaySpec);
    int indexOfRot = displaySpec.indexOf(":Rot");
    if (result && indexOfRot != -1) {
       int lastIndexOfRot = displaySpec.lastIndexOf(":");
       int t = qMin(displaySpec.mid(indexOfRot + 4, lastIndexOfRot - (indexOfRot + 4)).toUInt()/90, uint(Rot270));

        setTransformation(static_cast<Transformation>(t));
    }
    return result;
}


int QTransformedScreen::transformOrientation() const
{
    return (int)trans;
}

void QTransformedScreen::setTransformation(Transformation t)
{
    trans = t;
    QSize s = mapFromDevice(QSize(dw, dh));
    w = s.width();
    h = s.height();

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::setTransformation" << t << "size" << w << h << "dev size" << dw << dh;
#endif

}

static inline QRect correctNormalized(const QRect &r) {
    const int x1 = qMin(r.left(), r.right());
    const int x2 = qMax(r.left(), r.right());
    const int y1 = qMin(r.top(), r.bottom());
    const int y2 = qMax(r.top(), r.bottom());

    return QRect( QPoint(x1,y1), QPoint(x2,y2) );
}

static inline QMatrix blitMatrix(QTransformedScreen::Transformation t)
{
    QMatrix m;
    switch(t) {
    case QTransformedScreen::None:
        break;
    case QTransformedScreen::Rot270:
        m = QMatrix(0,1,-1,0, 0, 0);
        break;
    case QTransformedScreen::Rot180:
        m = QMatrix(-1, 0, 0, -1, 0, 0);
        break;
    case QTransformedScreen::Rot90:
        m = QMatrix(0, -1, 1, 0, 0, 0);
        break;
    }
    return m;
}


void QTransformedScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &region)
{
    if ( trans == None) {
        QT_TRANS_SCREEN_BASE::blit(img, topLeft, region);
        return;
    }
    //### slow but correct implementation; should be optimized later
    QRegion tr = mapToDevice(region, QSize(w,h));
    QRect imgRect(topLeft, img.size());
    QPoint tp = mapToDevice(imgRect, QSize(w,h)).topLeft();
    QImage ti = img.transformed(blitMatrix(trans));

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::blit region" << region << "transformed" << tr << "tr img size" << ti.size();;
#endif

    QT_TRANS_SCREEN_BASE::blit(ti, tp, tr);
}

void QTransformedScreen::solidFill(const QColor &color, const QRegion &region)
{
    QRegion tr = mapToDevice(region, QSize(w,h));

    Q_ASSERT(tr.boundingRect() == mapToDevice(region.boundingRect(), QSize(w,h)));

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::solidFill region" << region << "transformed" << tr;
#endif
    QT_TRANS_SCREEN_BASE::solidFill(color, tr);
}


// Mapping functions
QSize QTransformedScreen::mapToDevice(const QSize &s) const
{
    switch (trans) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
    return s;
}

QSize QTransformedScreen::mapFromDevice(const QSize &s) const
{
    switch (trans) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
        return s;
}

QPoint QTransformedScreen::mapToDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (trans) {
    case None:
        break;
    case Rot90:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    }

    return rp;
}

QPoint QTransformedScreen::mapFromDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (trans) {
    case None:
        break;
    case Rot90:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    }

    return rp;
}





QRect QTransformedScreen::mapToDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (trans) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    }

    return correctNormalized(tr);
}

QRect QTransformedScreen::mapFromDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (trans) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    }

    return correctNormalized(tr);
}

#if 1
QRegion QTransformedScreen::mapToDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;

#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice size" << s << "rgn:  " << rgn;
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (trans) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice trgn:  " << trgn;
#endif
    return trgn;
}

QRegion QTransformedScreen::mapFromDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (trans) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
#endif
    return trgn;
}
#endif

#endif // QT_NO_QWS_TRANSFORMED
