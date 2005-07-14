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

#ifndef QKBDYOPY_QWS_H
#define QKBDYOPY_QWS_H

#include "QtGui/qkbd_qws.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KBD_YOPY

class QWSYopyKbPrivate;

class QWSYopyKeyboardHandler : public QWSKeyboardHandler
{
public:
    explicit QWSYopyKeyboardHandler(const QString&);
    virtual ~QWSYopyKeyboardHandler();

private:
    QWSYopyKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_YOPY

#endif // QKBDYOPY_QWS_H
