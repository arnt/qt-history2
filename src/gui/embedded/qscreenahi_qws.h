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

#ifndef QAHISCREEN_H
#define QAHISCREEN_H

#include <QtGui/qscreenlinuxfb_qws.h>

#ifndef QT_NO_QWS_AHI

#include <ahi.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QAhiScreenPrivate;

class QAhiScreen : public QScreen
{
public:
    QAhiScreen(int displayId);
    ~QAhiScreen();

    bool connect(const QString &displaySpec);
    void disconnect();
    bool initDevice();
    void shutdownDevice();
    void setMode(int width, int height, int depth);

    void blit(const QImage &image, const QPoint &topLeft,
              const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);

private:
    AhiDevCtx_t context;
    AhiSurf_t surface;
};

QT_END_HEADER

#endif // QT_NO_QWS_AHI
#endif // QAHISCREEN_H
