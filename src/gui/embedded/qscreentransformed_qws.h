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

#ifndef QSCREENTRANSFORMED_QWS_H
#define QSCREENTRANSFORMED_QWS_H

#include "QtGui/qscreenvfb_qws.h"
#include "QtGui/qscreenlinuxfb_qws.h"
#include "QtGui/qmatrix.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_TRANSFORMED

//#define QT_TRANS_SCREEN_BASE   QLinuxFbScreen
//#define QT_TRANS_CURSOR_BASE   QScreenCursor

#define QT_TRANS_SCREEN_BASE        QVFbScreen
#define QT_TRANS_PAINTENGINE_BASE   QRasterPaintEngine
#define QT_TRANS_PAINTENGINE_BASE_P QRasterPaintEnginePrivate
#define QT_TRANS_CURSOR_BASE        QVFbScreenCursor

class QTransformedScreen : public QT_TRANS_SCREEN_BASE
{
public:
    explicit QTransformedScreen(int display_id);

    enum Transformation
    {
        None   = 0,
        Rot90  = 90,
        Rot180 = 180,
        Rot270 = 270
    };
    bool connect(const QString &displaySpec);
    int initCursor(void *end_of_location, bool init = false);

    void setTransformation(Transformation t);
    Transformation transformation() const
    { return trans; }
    int transformOrientation() const;
    bool isTransformed() const
    { return trans != None; }
    const QMatrix &matrix();


    // Mapping functions
    QSize mapToDevice(const QSize &s) const;
    QSize mapFromDevice(const QSize &s) const;

    QPoint mapToDevice(const QPoint &, const QSize &) const;
    QPoint mapFromDevice(const QPoint &, const QSize &) const;
    QRect mapToDevice(const QRect &, const QSize &) const;
    QRect mapFromDevice(const QRect &, const QSize &) const;

#if 0
    QRegion mapToDevice(const QRegion &, const QSize &) const;
    QRegion mapFromDevice(const QRegion &, const QSize &) const;
#endif
    //QImage mapToDevice(const QImage &) const;
    //QImage mapFromDevice(const QImage &) const;

    QPaintEngine *createPaintEngine(unsigned char *addr,int w, int h, int d, int linestep);

private:
    QMatrix deltaCompensation(int deg);

    Transformation trans;
    QMatrix rotMatrix;
    QScreen *driver;
};

#endif // QT_NO_QWS_TRANSFORMED
#endif // QSCREENTRANSFORMED_QWS_H
