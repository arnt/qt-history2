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

#ifndef QKBDDRIVERFACTORY_QWS_H
#define QKBDDRIVERFACTORY_QWS_H

#include "QtCore/qstringlist.h"

QT_MODULE(Gui)

class QString;
class QWSKeyboardHandler;

class Q_GUI_EXPORT QKbdDriverFactory
{
public:
    static QStringList keys();
    static QWSKeyboardHandler *create(const QString&, const QString&);
};

#endif // QKBDDRIVERFACTORY_QWS_H
