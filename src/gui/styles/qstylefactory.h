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

#ifndef QSTYLEFACTORY_H
#define QSTYLEFACTORY_H

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QStyle;

class Q_GUI_EXPORT QStyleFactory
{
public:
    static QStringList keys();
    static QStyle *create(const QString&);
};

QT_END_HEADER

#endif // QSTYLEFACTORY_H
