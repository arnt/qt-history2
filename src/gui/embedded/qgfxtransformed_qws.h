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

#ifndef QGFXTRANSFORMED_QWS_H
#define QGFXTRANSFORMED_QWS_H

#include "QtGui/qscreen_qws.h"

#ifndef QT_NO_QWS_TRANSFORMED

#define QT_TRANS_SCREEN_BASE    QLinuxFbScreen
#define QT_TRANS_CURSOR_BASE        QScreenCursor
#define QT_TRANS_GFX_BASE        QGfxRaster
//#define QT_TRANS_SCREEN_BASE  QVFbScreen
//#define QT_TRANS_CURSOR_BASE   QVFbScreenCursor
//#define QT_TRANS_GFX_BASE      QGfxVFb
#include "QtGui/qgfxlinuxfb_qws.h"

class QTransformedScreen : public QT_TRANS_SCREEN_BASE
{
public:
    explicit QTransformedScreen(int display_id);
    virtual ~QTransformedScreen();

    virtual bool connect(const QString &displaySpec);
    virtual int initCursor(void* e, bool init);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

    enum Transformation { None, Rot90, Rot180, Rot270 };
    Transformation transformation() const { return trans; }

    virtual bool isTransformed() const;
    virtual QSize mapToDevice(const QSize &) const;
    virtual QSize mapFromDevice(const QSize &) const;
    virtual QPoint mapToDevice(const QPoint &, const QSize &) const;
    virtual QPoint mapFromDevice(const QPoint &, const QSize &) const;
    virtual QRect mapToDevice(const QRect &, const QSize &) const;
    virtual QRect mapFromDevice(const QRect &, const QSize &) const;
    virtual QImage mapToDevice(const QImage &) const;
    virtual QImage mapFromDevice(const QImage &) const;
    virtual QRegion mapToDevice(const QRegion &, const QSize &) const;
    virtual QRegion mapFromDevice(const QRegion &, const QSize &) const;
    virtual int transformOrientation() const;

    void setTransformation(Transformation t);

private:
    Transformation trans;
    QScreen *driver;
};

#endif // QT_NO_QWS_TRANSFORMED

#endif // QGFXTRANSFORMED_QWS_H
