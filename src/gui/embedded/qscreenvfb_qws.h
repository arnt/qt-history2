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

#ifndef QSCREENVFB_QWS_H
#define QSCREENVFB_QWS_H

#include <QtGui/qscreen_qws.h>
#include <QtGui/qvfbhdr.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_QVFB

class QVFbScreen : public QScreen
{
public:
    explicit QVFbScreen(int display_id);
    virtual ~QVFbScreen();
    virtual bool initDevice();
    virtual bool connect(const QString &displaySpec);
    virtual void disconnect();
    virtual void shutdownDevice();
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    virtual void setDirty(const QRect& r)
        { hdr->dirty = true; hdr->update = hdr->update.unite(r); }

    bool success;
    unsigned char *shmrgn;
    QVFbHeader *hdr;
};

#endif // QT_NO_QWS_QVFB

QT_END_HEADER

#endif // QSCREENVFB_QWS_H
