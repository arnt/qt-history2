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

#ifndef QMULTISCREEN_QWS_P_H
#define QMULTISCREEN_QWS_P_H

#include <qscreen_qws.h>

class QMultiScreenPrivate;

class QMultiScreen : public QScreen
{
public:
    QMultiScreen(int displayId);
    ~QMultiScreen();
    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();
    void setMode(int,int,int);
    bool supportsDepth(int) const;

    void save();
    void restore();
    void blank(bool on);

    bool onCard(const unsigned char *) const;
    bool onCard(const unsigned char *, ulong& out_offset) const;

#if 0
    QSize mapToDevice(const QSize &) const;
    QSize mapFromDevice(const QSize &) const;
    QPoint mapToDevice(const QPoint &, const QSize &) const;
    QPoint mapFromDevice(const QPoint &, const QSize &) const;
    QRect mapToDevice(const QRect &, const QSize &) const;
    QRect mapFromDevice(const QRect &, const QSize &) const;
    QImage mapToDevice(const QImage &) const;
    QImage mapFromDevice(const QImage &) const;
    QRegion mapToDevice(const QRegion &, const QSize &) const;
    QRegion mapFromDevice(const QRegion &, const QSize &) const;
#endif

    bool isInterlaced() const;

    int memoryNeeded(const QString&);
    int sharedRamSize(void *);

    void haltUpdates();
    void resumeUpdates();

    void exposeRegion(QRegion r, int changing);

    void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);
    void blit(QWSWindow *bs, const QRegion &clip);
    void setDirty(const QRect&);

    QWSWindowSurface* createSurface(QWidget *widget) const;

    QList<QScreen*> subScreens() const;
    QRegion region() const;

private:
    void addSubScreen(QScreen *screen);
    void removeSubScreen(QScreen *screen);

    QMultiScreenPrivate *d_ptr;
};

#endif // QMULTISCREEN_QWS_P_H
