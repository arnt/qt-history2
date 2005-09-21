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

#ifndef QGLBUFFER_P_H
#define QGLBUFFER_P_H

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

#ifdef Q_WS_X11
#include <GL/glx.h>
#ifndef GLX_VERSION_1_3
typedef XID GLXPbuffer;
#endif
#elif defined(Q_WS_WIN)
DECLARE_HANDLE(HPBUFFERARB);
#elif defined(Q_WS_MACX)
#include <AGL/agl.h>
#endif

class QGLPbufferPrivate {
    Q_DECLARE_PUBLIC(QGLPbuffer)
public:
    QGLPbufferPrivate() : invalid(true), qctx(0), pbuf(0), ctx(0)
    {
#ifdef Q_WS_WIN
        dc = 0;
#elif defined(Q_WS_MACX)
        share_ctx = 0;
#endif
    }

    bool invalid;
    QSize size;
    QGLContext *qctx;
    QGLPbuffer *q_ptr;

#ifdef Q_WS_X11
    GLXPbuffer pbuf;
    GLXContext ctx;
#elif defined(Q_WS_WIN)
    QGLWidget dmy;
    HDC dc;
    HPBUFFERARB pbuf;
    HGLRC ctx;
#elif defined(Q_WS_MACX)
    AGLPbuffer pbuf;
    AGLContext ctx;
    AGLContext share_ctx;
#endif
};

#endif // QGLBUFFER_P_H
