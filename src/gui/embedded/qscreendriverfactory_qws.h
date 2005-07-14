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

#ifndef QSCREENDRIVERFACTORY_QWS_H
#define QSCREENDRIVERFACTORY_QWS_H

#include "QtCore/qstringlist.h"

QT_MODULE(Gui)

class QString;
class QScreen;

class Q_GUI_EXPORT QScreenDriverFactory
{
public:
    static QStringList keys();
    static QScreen *create(const QString&, int);
};

#endif //QSCREENDRIVERFACTORY_QWS_H
