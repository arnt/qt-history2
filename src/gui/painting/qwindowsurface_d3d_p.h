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

#ifndef QWINDOWSURFACE_D3D_P_H
#define QWINDOWSURFACE_D3D_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qglobal.h>
#include "private/qwindowsurface_p.h"

class QPaintDevice;
class QPoint;
class QRegion;
class QWidget;
struct QD3DWindowSurfacePrivate;

class QD3DWindowSurface : public QWindowSurface
{
public:
    QD3DWindowSurface(QWidget *widget);
    ~QD3DWindowSurface();

    QPaintDevice *paintDevice();
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void setGeometry(const QRect &rect);
    bool scroll(const QRegion &area, int dx, int dy);

private:
    QD3DWindowSurfacePrivate *d_ptr;
};

#endif // QWINDOWSURFACE_D3D_P_H
