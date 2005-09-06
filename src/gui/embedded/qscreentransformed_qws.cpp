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

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <qwindowsystem_qws.h>

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
// Transformed Cursor
// -------------------------------------------------------------------------------------------------
#ifndef QT_NO_QWS_CURSOR
class QTransformedScreenCursor : public QT_TRANS_CURSOR_BASE
{
public:
    QTransformedScreenCursor(QTransformedScreen *s);
    void set(const QImage &image, int hotx, int hoty);
};

QTransformedScreenCursor::QTransformedScreenCursor(QTransformedScreen *s)
    : QT_TRANS_CURSOR_BASE((QT_TRANS_SCREEN_BASE *)s)
{
}

void QTransformedScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    double dx2 = image.width() / 2.0;
    double dy2 = image.height() / 2.0;
    // Create rotation matrix
    QMatrix rotMatrix;
    rotMatrix.translate(dx2, dy2);
    rotMatrix.rotate(qt_trans_screen->transformOrientation());
    rotMatrix.translate(-dx2, -dy2);
    // Rotate image and hot-point   // ### Why do we require 8-bit cursors?
    QImage rimg = image.transformed(rotMatrix, Qt::SmoothTransformation).convertToFormat(QImage::Format_Indexed8);
    QPoint tp = rotMatrix.map(QPoint(hotx, hoty));
    // Set it using the base class
    QT_TRANS_CURSOR_BASE::set(rimg, tp.x(), tp.y());
}
#endif

// -------------------------------------------------------------------------------------------------
// Transformed Screen
// -------------------------------------------------------------------------------------------------
QTransformedScreen::QTransformedScreen(int display_id)
    : QT_TRANS_SCREEN_BASE(display_id)
{
    setTransformation(None);
    qt_trans_screen = this;
}

bool QTransformedScreen::connect(const QString &displaySpec)
{
    bool result = QT_TRANS_SCREEN_BASE::connect(displaySpec);
    int indexOfRot = displaySpec.indexOf(":Rot");
    if (result && indexOfRot != -1)
        setTransformation((Transformation)displaySpec.mid(indexOfRot + 4).toInt());

    return result;
}

/*
    The end_of_location parameter is unusual: it's the address
    after the cursor data.
*/
int QTransformedScreen::initCursor(void *end_of_location, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor = true;
    SWCursorData *data_eol = (SWCursorData *)end_of_location - 1;
    qt_screencursor = new QTransformedScreenCursor(this);
    qt_screencursor->init(data_eol, init);
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

int QTransformedScreen::transformOrientation() const
{
    return (int)trans;
}

void QTransformedScreen::setTransformation(Transformation t)
{
    trans = t;
    matrix(); // force matrix recalculation

    QSize s = mapFromDevice(QSize(dw, dh));
    w = s.width();
    h = s.height();
}

QMatrix QTransformedScreen::deltaCompensation(int deg)
{
    QMatrix corr;

    int diff = dw - dh;
    if (diff > 0) { // Landscape
        if (deg > 270)
            corr.translate(0.0, (-1.0 + (deg - 270)/90.0) * diff);
        else if (deg > 180)
            corr.translate(0.0, -diff);
        else if (deg > 90)
            corr.translate(0.0, -(deg - 90)/90.0 * diff);

    } else if (diff < 0) { // Portrait
        if (deg < 90)
            corr.translate((deg/90.0) * diff, 0.0);
        else if (deg < 180)
            corr.translate(diff, 0.0);
        else if (deg < 270)
            corr.translate((1.0 - (deg - 180)/90.0) * diff, 0.0);
    }

    // Could also have been written combined as:
    //     if (diff) {
    //         if (deg < 90)
    //             corr.translate(qMin(0.0, (deg/90.0) * diff), 0.0);
    //         else if (deg < 180)
    //             corr.translate(qMin(0, diff), qMin(0.0, -(deg - 90)/90.0 * diff));
    //         else if (deg < 270)
    //             corr.translate(qMin(0.0, (1.0 - (deg - 180)/90.0) * diff), qMin(0, -diff));
    //         else if (deg < 360)
    //             corr.translate(0.0, qMin(0.0, (-1.0 + (deg - 270)/90.0) * diff));
    //     }
    // but split preferred to lower the number of calculations for a given device orientation.

    return corr;
}

const QMatrix &QTransformedScreen::matrix()
{
    static Transformation prevTrans = None;
    if (prevTrans == trans)
        return rotMatrix;
    prevTrans = trans;

    // Update screen matrix
    rotMatrix.reset();
    switch(trans) {
    case QTransformedScreen::None:
        break;
    case QTransformedScreen::Rot90:
        rotMatrix = QMatrix(0,1,-1,0, dw, 0);
        break;
    case QTransformedScreen::Rot180:
        rotMatrix = QMatrix(-1, 0, 0, -1, dw, dh);
        break;
    case QTransformedScreen::Rot270:
        rotMatrix = QMatrix(0, -1, 1, 0, 0, dh);
        break;
    default:
        {   // Free rotation, only works for 0 - 360 degrees:
            // General purpose rotation, can be used as transition
            // effect to one of the basic rotations above.
            double center = qMax(dw, dh) / 2.0;
            rotMatrix.translate(center, center);
            rotMatrix.rotate(trans);
            rotMatrix.translate(-center, -center);
            rotMatrix *= deltaCompensation(trans); // gradually move origo
        }
        break;
    }
    return rotMatrix;
}


// Mapping functions
QSize QTransformedScreen::mapToDevice(const QSize &s) const
{
    return rotMatrix.mapRect(QRect(QPoint(0,0), s)).size();
}

QSize QTransformedScreen::mapFromDevice(const QSize &s) const
{
    return rotMatrix.inverted().mapRect(QRect(QPoint(0,0), s)).size();
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
    default:
        rp = rotMatrix.map(p);
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
    default:
        rp = rotMatrix.inverted().map(p);
        break;
    }

    return rp;
}

QRect QTransformedScreen::mapToDevice(const QRect &r, const QSize &s) const
{
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
    default:
        tr = rotMatrix.map(r);
        break;
    }

    return tr.normalized();
}

QRect QTransformedScreen::mapFromDevice(const QRect &r, const QSize &s) const
{
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
    default:
        tr = rotMatrix.inverted().map(r);
        break;
    }

    return tr.normalized();
}

#if 0
QRegion QTransformedScreen::mapToDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;

    qDebug() << "toDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
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
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= tr.normalized();
        }
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= tr.normalized();
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= tr.normalized();
        }
        break;
    default:
        trgn = rotMatrix.map(rgn);
        QPixmap foo(dw*2, dh*2);
        QPainter p(&foo);
        p.fillRect(0,0,dw*2,dh*2, Qt::white);
        p.translate(dw/2, dh/2);
        p.setClipRegion(rgn);
        p.fillRect(-dw,-dh,dw*2,dh*2, Qt::green);
        p.setClipRegion(trgn);
        p.fillRect(-dw,-dh,dw*2,dh*2, Qt::red);
        p.end();
        foo.save("to.png", "PNG");
        break;
    }

    qDebug() << "toDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
    return trgn;
}

QRegion QTransformedScreen::mapFromDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;

    qDebug() << "fromDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
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
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= tr.normalized();
        }
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= tr.normalized();
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= tr.normalized();
        }
        break;
    default:
        trgn = rotMatrix.inverted().map(rgn);
        QPixmap foo(dw*2, dh*2);
        QPainter p(&foo);
        p.fillRect(0,0,dw*2,dh*2, Qt::white);
        p.translate(dw/2, dh/2);
        p.setClipRegion(rgn);
        p.fillRect(-dw,-dh,dw*2,dh*2, Qt::green);
        p.setClipRegion(trgn);
        p.fillRect(-dw,-dh,dw*2,dh*2, Qt::red);
        p.end();
        foo.save("from.png", "PNG");
        break;
    }

    qDebug() << "fromDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
    return trgn;
}
#endif

#endif // QT_NO_QWS_TRANSFORMED
