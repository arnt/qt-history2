/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWINDOWSURFACE_P_H
#define QWINDOWSURFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qwidget.h>

class QPaintDevice;
class QRegion;
class QRect;
class QPoint;
class QImage;
class QWindowSurfacePrivate;

class Q_GUI_EXPORT QWindowSurface
{
public:
    QWindowSurface(QWidget *window);
    virtual ~QWindowSurface();

    QWidget *window() const;

    virtual QPaintDevice *paintDevice() = 0;
    virtual void flush(QWidget *widget, const QRegion &region,
                       const QPoint &offset) = 0;
    virtual void setGeometry(const QRect &rect);
    QRect geometry() const;

    virtual bool scroll(const QRegion &area, int dx, int dy);

    virtual void beginPaint(const QRegion &);
    virtual void endPaint(const QRegion &);

    virtual QImage* buffer(const QWidget *widget);
    virtual QPoint offset(const QWidget *widget) const;
    inline QRect rect(const QWidget *widget) const;

private:
    QWindowSurfacePrivate *d_ptr;
};

QRect QWindowSurface::rect(const QWidget *widget) const
{
    return widget->rect().translated(offset(widget));
}

#endif // QWINDOWSURFACE_P_H
