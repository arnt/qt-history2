/****************************************************************************
**
** Implementation of QTextCodecFactory class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextcodecfactory.h"

#ifndef QT_NO_TEXTCODEC

#ifndef QT_NO_COMPONENT
#include "qcoreapplication.h"
#include "qcleanuphandler.h"
#include <private/qpluginmanager_p.h>
#include "qtextcodecinterface_p.h"

#include <private/qmutexpool_p.h>

#include <stdlib.h>


static QPluginManager<QTextCodecFactoryInterface> *manager = 0;
static QSingleCleanupHandler< QPluginManager<QTextCodecFactoryInterface> > cleanup_manager;

static void create_manager()
{
    if ( manager ) // already created
	return;

    // protect manager creation
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &manager ) : 0);

    // we check the manager pointer again to make sure that another thread
    // has not created the manager before us.

    if ( manager ) // already created
	return;

    manager =
	new QPluginManager<QTextCodecFactoryInterface>(IID_QTextCodecFactory,
						       QCoreApplication::libraryPaths(), "/codecs",
						       FALSE);
    cleanup_manager.set( &manager );
}

#endif // QT_NO_COMPONENT


QTextCodec *QTextCodecFactory::createForName(const QString &name)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    // make sure the manager is created
    create_manager();

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    manager->queryInterface(name, &iface );

    if (iface)
	codec = iface->createForName(name);

#endif // QT_NO_COMPONENT

    return codec;
}


QTextCodec *QTextCodecFactory::createForMib(int mib)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    // make sure the manager is created
    create_manager();

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    manager->queryInterface("MIB-" + QString::number(mib), &iface );

    if (iface)
	codec = iface->createForMib(mib);

#endif // QT_NO_COMPONENT

    return codec;
}


#endif // QT_NO_TEXTCODEC
