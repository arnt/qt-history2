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

#ifndef QGFXREPEATER_QWS_H
#define QGFXREPEATER_QWS_H

#include "QtGui/qscreen_qws.h"

#ifndef QT_NO_QWS_REPEATER

#include "QtGui/qgfx_qws.h"
#include "QtCore/qlist.h"

class QScreenRec;

class QRepeaterScreen : public QScreen
{
public:

    explicit QRepeaterScreen(int);
    virtual ~QRepeaterScreen();

    virtual bool connect(const QString &);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual bool initDevice();
    virtual void disconnect() {}
    virtual void setMode(int,int,int) {}
    virtual int initCursor(void *,bool=false);
    virtual void setDirty(const QRect &);
    virtual int sharedRamSize(void *);
    QRegion getRequiredUpdate(int,int,int,int,int,int);

private:

    bool sw_cursor_exists;

    QList<QScreenRec*> screens;

};

#endif // QT_NO_QWS_REPEATER

#endif // QGFXREPEATER_QWS_H
