/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXREPEATER_QWS_H
#define QGFXREPEATER_QWS_H

#ifndef QT_H
#include "qgfx_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_REPEATER

#include "qlist.h"

class QScreenRec;

class QRepeaterScreen : public QScreen
{
public:

    QRepeaterScreen(int);
    virtual ~QRepeaterScreen();

    virtual bool connect(const QString &);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual bool initDevice();
    virtual void disconnect() {}
    virtual void setMode(int,int,int) {}
    virtual int initCursor(void *,bool=false);
    virtual void setDirty(const QRect &);
    virtual int sharedRamSize(void *);
    QImage * readScreen(int,int,int,int,QRegion &);
    QRegion getRequiredUpdate(int,int,int,int,int,int);

private:

    bool sw_cursor_exists;

    QList<QScreenRec*> screens;

};

#endif // QT_NO_QWS_REPEATER

#endif // QGFXREPEATER_QWS_H
