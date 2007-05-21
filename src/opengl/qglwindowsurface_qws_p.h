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

#ifndef QWINDOWSURFACE_GL_P_H
#define QWINDOWSURFACE_GL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QWSGLWindowSurface class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QPaintDevice>
#include "private/qwindowsurface_qws_p.h"

class QPaintDevice;
class QPoint;
class QRegion;
class QSize;
class QWidget;
class QGLContext;

class QWSGLWindowSurfacePrivate;

class Q_OPENGL_EXPORT QWSGLWindowSurface : public QWSWindowSurface
{
    Q_DECLARE_PRIVATE(QWSGLWindowSurface)

public:
    QWSGLWindowSurface(QWidget *widget);
    QWSGLWindowSurface();
    ~QWSGLWindowSurface();

    QGLContext *context() const;
    void setContext(QGLContext *context);

private:
    QWSGLWindowSurfacePrivate *d_ptr;
};


#endif // QWINDOWSURFACE_GL_P_H
