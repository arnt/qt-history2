/****************************************************************************
**
** Implementation of QRemoteFactory class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qremoteinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qremotefactory.h"

#ifndef QT_NO_REMOTE

#include "qapplication.h"
#include <private/qpluginmanager_p.h>
#include <stdlib.h>

#ifndef QT_NO_COMPONENT
class QRemoteFactoryPrivate : public QObject
{
public:
    QRemoteFactoryPrivate();
    ~QRemoteFactoryPrivate();

    static QPluginManager<QRemoteFactoryInterface> *manager;
};

static QRemoteFactoryPrivate *instance = 0;
QPluginManager<QRemoteFactoryInterface> *QRemoteFactoryPrivate::manager = 0;

QRemoteFactoryPrivate::QRemoteFactoryPrivate()
: QObject( qApp )
{
    manager = new QPluginManager<QRemoteFactoryInterface>( IID_QRemoteFactory, QApplication::libraryPaths(), "/remote", FALSE );
}

QRemoteFactoryPrivate::~QRemoteFactoryPrivate()
{
    delete manager;
    manager = 0;

    instance = 0;
}

#endif //QT_NO_COMPONENT

/*!
  \class QRemoteFactory QRemoteFactory.h
  \brief The QRemoteFactory class creates QRemoteInterface objects.

  The remote factory creates a QRemoteInterface object for a given key with
  QRemoteFactory::create(key).

  The Remote-controls are either built-in or dynamically loaded from a RC
  plugin.

  QRemoteFactory::keys() returns a list of valid keys. Qt currently
  ships with "rc1".

*/

/*!  Creates a QRemoteInterface object that matches \a key. Currently this is
  always a remote control from a plugin.

  \sa keys()
*/
QRemoteInterface *QRemoteFactory::create( const QString& key )
{
    QString c = key.lower();

#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QRemoteFactoryPrivate;

    QInterfacePtr<QRemoteFactoryInterface> iface;
    QRemoteFactoryPrivate::manager->queryInterface( c, &iface );

    if ( iface )
	return iface->create( c );

#endif
    return 0;
}

#ifndef QT_NO_STRINGLIST
/*!
  Returns the list of keys  this factory can create
  remote controls for.

  \sa create()
*/
QStringList QRemoteFactory::keys()
{
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QRemoteFactoryPrivate;

    QStringList list = QRemoteFactoryPrivate::manager->featureList();
#else
    QStringList list;
#endif //QT_NO_COMPONENT

    if ( !list.contains( "rc1" ) )
	list << "rc1";

    return list;
}
#endif // QT_NO_STRINGLIST

#endif // QT_NO_REMOTE
