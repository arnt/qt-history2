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

#ifndef QMOUSEDRIVERFACTORY_QWS_H
#define QMOUSEDRIVERFACTORY_QWS_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QWSMouseHandler;

class Q_GUI_EXPORT QMouseDriverFactory
{
public:
    static QStringList keys();
    static QWSMouseHandler *create(const QString&, const QString &);
};

#endif //QMOUSEDRIVERFACTORY_QWS_H
