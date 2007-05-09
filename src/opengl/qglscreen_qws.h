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

#ifndef QSCREENEGL_P_H
#define QSCREENEGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QScreenEGL class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QScreen>
#include <QtOpenGL/qgl.h>

QT_BEGIN_HEADER

QT_MODULE(OpenGL)

class QGLScreenPrivate;

class QGLScreen : public QScreen
{
    Q_DECLARE_PRIVATE(QGLScreen)
public:
    QGLScreen(int displayId);
    virtual ~QGLScreen();

    virtual bool chooseContext(QGLContext *context, const QGLContext *shareContext);
    virtual bool hasOpenGL() = 0;
private:
    QGLScreenPrivate *d_ptr;
};

QT_END_HEADER

#endif // QSCREENEGL_P_H
