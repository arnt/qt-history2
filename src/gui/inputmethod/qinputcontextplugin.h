/****************************************************************************
** $Id$
**
** Definition of QInputContextPlugin class
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

#ifndef QINPUTCONTEXTPLUGIN_H
#define QINPUTCONTEXTPLUGIN_H

#ifndef QT_H
#include "qplugin.h"
#include "qfactoryinterface.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_IM
class QInputContext;
class QInputContextPluginPrivate;

struct Q_GUI_EXPORT QInputContextFactoryInterface : public QFactoryInterface
{
    virtual QInputContext *create( const QString &key ) = 0;
    virtual QStringList languages( const QString &key ) = 0;
    virtual QString displayName( const QString &key ) = 0;
    virtual QString description( const QString &key ) = 0;
};

Q_DECLARE_INTERFACE(QInputContextFactoryInterface, "http://trolltech.com/Qt/QInputContextFactoryInterface")

class Q_GUI_EXPORT QInputContextPlugin : public QObject, public QInputContextFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QInputContextFactoryInterface:QFactoryInterface)
public:
    QInputContextPlugin(QObject *parent = 0);
    ~QInputContextPlugin();

    virtual QStringList keys() const = 0;
    virtual QInputContext *create( const QString &key ) = 0;
    virtual QStringList languages( const QString &key ) = 0;
    virtual QString displayName( const QString &key ) = 0;
    virtual QString description( const QString &key ) = 0;
};
#endif // QT_NO_IM
#endif // QINPUTCONTEXTPLUGIN_H
