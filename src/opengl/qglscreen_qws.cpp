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

#include <QGLScreen>
#include <QGLContext>
#include <QGLWidget>
#include "private/qglwindowsurface_qws_p.h"

class QGLScreenPrivate
{

};

QGLScreen::QGLScreen(int displayId)
    : QScreen(displayId), d_ptr(new QGLScreenPrivate)
{
}

QGLScreen::~QGLScreen()
{
    delete d_ptr;
}

/*!
    \since 4.3

    Initializes the \a context and sets up the QGLWindowSurface of the QWidget of \a context
    based on the parameters of \a context and based on its own requirements.
    The format() of \a context needs to be updated with the
    actual parameters of the OpenGLES drawable that was set up.

    \a shareContext is used in the same way as for QGLContext. It is the context with which \a context
    shares display lists and texture ids etc. The window surface must be set up so that this sharing works.

    returns true in case of success and false if it is not possible to create the necessary OpenGLES
    drawable/context.
*/
bool QGLScreen::chooseContext(QGLContext *context, const QGLContext *shareContext)
{
    Q_UNUSED(shareContext);

    QGLWidget *widget = static_cast<QGLWidget*>(context->device());
    if (context->device()->devType() == QInternal::Widget) {
        QGLWindowSurface *surface = static_cast<QGLWindowSurface*>(widget->windowSurface());
        surface->setContext(context);
        return true;
    }
    return false;
}
