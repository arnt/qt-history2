/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCREENTRANSFORMED_QWS_H
#define QSCREENTRANSFORMED_QWS_H

#include <QtGui/qscreenlinuxfb_qws.h>
#include <QtGui/qscreenvfb_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_TRANSFORMED

#ifdef QT_NO_QWS_QVFB
#define QT_TRANS_SCREEN_BASE   QLinuxFbScreen
#else
#define QT_TRANS_SCREEN_BASE   QVFbScreen
#endif

#ifdef qdoc
class QTransformedScreen : public QScreen
#else
class QTransformedScreen : public QT_TRANS_SCREEN_BASE
#endif
{
public:
    explicit QTransformedScreen(int display_id);

    enum Transformation { None, Rot90, Rot180, Rot270 };
    bool connect(const QString &displaySpec);

    void setTransformation(Transformation t);
    Transformation transformation() const
    { return trans; }
    int transformOrientation() const;
    bool isTransformed() const
    { return trans != None; }

    // drawing functions
    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);


    // Mapping functions
    QSize mapToDevice(const QSize &s) const;
    QSize mapFromDevice(const QSize &s) const;

    QPoint mapToDevice(const QPoint &, const QSize &) const;
    QPoint mapFromDevice(const QPoint &, const QSize &) const;
    QRect mapToDevice(const QRect &, const QSize &) const;
    QRect mapFromDevice(const QRect &, const QSize &) const;


    QRegion mapToDevice(const QRegion &, const QSize &) const;
    QRegion mapFromDevice(const QRegion &, const QSize &) const;

    //QImage mapToDevice(const QImage &) const;
    //QImage mapFromDevice(const QImage &) const;

private:
    Transformation trans;
};

#endif // QT_NO_QWS_TRANSFORMED

QT_END_HEADER

#endif // QSCREENTRANSFORMED_QWS_H
