/****************************************************************************
** $Id$
**
** Definition of QInputContextFactory class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

/****************************************************************************
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the input method module of the Qt Toolkit.
**
** Licensees holding valid Qt Preview licenses may use this file in
** accordance with the Qt Preview License Agreement provided with the
** Software.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QINPUTCONTEXTFACTORY_H
#define QINPUTCONTEXTFACTORY_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_IM

class QInputContext;
class QWidget;

class Q_GUI_EXPORT QInputContextFactory
{
public:
    static QStringList keys();
    static QInputContext *create( const QString &key, QObject *parent ); // should be a toplevel widget
    static QStringList languages( const QString &key );
    static QString displayName( const QString &key );
    static QString description( const QString &key );
};
#endif //QT_NO_IM

#endif //QINPUTCONTEXTFACTORY_H
