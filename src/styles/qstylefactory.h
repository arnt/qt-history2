/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTYLEFACTORY_H
#define QSTYLEFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class QString;
class QStyle;

class Q_GUI_EXPORT QStyleFactory
{
public:
#ifndef QT_NO_STRINGLIST
    static QStringList keys();
#endif
    static QStyle *create( const QString& );
};

#endif //QT_NO_STYLE

#endif //QSTYLEFACTORY_H
