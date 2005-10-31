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

#ifndef QSCREENVFB_QWS_H
#define QSCREENVFB_QWS_H

#include <QtGui/qscreen_qws.h>
#include <QtGui/qvfbhdr.h>

QT_MODULE(Gui)

#ifndef QT_NO_QWS_QVFB

class QVFbMouseHandler;
class QVFbKeyboardHandler;

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
    QVFbMouseHandler *mouseHandler;
    QVFbKeyboardHandler *keyboardHandler;
};

#endif // QT_NO_QWS_QVFB

#endif // QSCREENVFB_QWS_H
