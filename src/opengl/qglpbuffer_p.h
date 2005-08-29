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

#ifndef QGLPBUFFER_P_H
#define QGLPBUFFER_P_H

#ifdef Q_WS_X11
#include <GL/glx.h>

#ifndef GLXPbuffer
typedef XID GLXPbuffer;
#endif

class QGLPbufferPrivate {
    Q_DECLARE_PUBLIC(QGLPbuffer)
public:
    QGLPbufferPrivate() : pbuf(0), ctx(0), invalid(true), paintEngine(0) {}

    GLXPbuffer pbuf;
    GLXContext ctx;
    bool invalid;
    QSize size;
    QGLPbuffer *q_ptr;
    QPaintEngine *paintEngine;
};
#elif defined(Q_WS_WIN)
DECLARE_HANDLE(HPBUFFERARB);

class QGLPbufferPrivate {
    Q_DECLARE_PUBLIC(QGLPbuffer)
public:
    QGLPbufferPrivate() : invalid(true), dc(0) {}

    bool invalid;
    QSize size;
    QGLPbuffer *q_ptr;
    QGLWidget dmy;
    HDC dc;
    HPBUFFERARB pbuf;
    HGLRC ctx;
    QPaintEngine *paintEngine;
};
#elif defined(Q_WS_MACX)
#include <AGL/agl.h>

class QGLPbufferPrivate {
    Q_DECLARE_PUBLIC(QGLPbuffer)
public:
    QGLPbufferPrivate() : invalid(true) {}

    bool invalid;
    QSize size;
    QGLPbuffer *q_ptr;
    AGLPbuffer pbuf;
    AGLContext ctx;
    AGLContext share_ctx;
    QPaintEngine *paintEngine;
};
#endif // Q_WS_

#endif // QGLPBUFFER_P_H
