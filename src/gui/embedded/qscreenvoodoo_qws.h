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

#ifndef QGFXVOODOO_QWS_H
#define QGFXVOODOO_QWS_H

#include "QtGui/qscreenlinuxfb_qws.h"

#ifndef QT_NO_QWS_VOODOO

class QVoodooScreen : public QLinuxFbScreen
{
public:
    explicit QVoodooScreen(int display_id);
    virtual ~QVoodooScreen();

    virtual bool connect(const QString &spec);
    virtual bool initDevice();
    virtual void shutdownDevice();
    virtual int initCursor(void *,bool);
    virtual bool useOffscreen();

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

    unsigned char * voodoo_regbase;
};

#endif // QT_NO_QWS_VOODOO

#endif // QGFXVOODOO_QWS_H
