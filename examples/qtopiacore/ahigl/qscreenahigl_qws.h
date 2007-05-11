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

#ifndef QAHIGLSCREEN_H
#define QAHIGLSCREEN_H

#include <QGLScreen>
#include <QWSServer>

class QAhiGLScreenPrivate;

class QAhiGLScreen : public QGLScreen
{
public:
    QAhiGLScreen(int displayId);
    virtual ~QAhiGLScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();

    void setMode(int width, int height, int depth);
    void blank(bool on);

    void exposeRegion(QRegion r, int changing);

    QWSWindowSurface* createSurface(QWidget *widget) const;
    QWSWindowSurface* createSurface(const QString &key) const;

    bool hasOpenGL();

private:
    void updateTexture(int windowIndex);
    void redrawScreen();
    void drawWindow(QWSWindow *win, qreal progress);
    void drawQuad(const QRect &textureGeometry,
                  const QRect &subGeometry,
                  const QRect &screenGeometry);
    void drawQuadInverseY(const QRect &textureGeometry,
                          const QRect &subTexGeometry,
                          const QRect &screenGeometry);
    void drawQuadWavyFlag(const QRect &textureGeometry,
                          const QRect &subTexGeometry,
                          const QRect &screenGeometry,
                          float progress);

    QAhiGLScreenPrivate *d_ptr;
    friend class QAhiGLScreenPrivate;
};

#endif // QAHIGLSCREEN_H
