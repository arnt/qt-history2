/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKBDDRIVERFACTORY_QWS_H
#define QKBDDRIVERFACTORY_QWS_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

class QString;
class QWSKeyboardHandler;

class Q_GUI_EXPORT QKbdDriverFactory
{
public:
    static QStringList keys();
    static QWSKeyboardHandler *create( const QString&, const QString& );
};

#endif //QKBDDRIVERFACTORY_QWS_H
