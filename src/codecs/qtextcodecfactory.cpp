/****************************************************************************
**
** Implementation of QTextCodecFactory class
**
** Created : 010130
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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

#include "qtextcodecfactory.h"

#ifndef QT_NO_TEXTCODEC

#ifndef QT_NO_COMPONENT
#include "qapplication.h"
#include <private/qpluginmanager_p.h>
#include "qtextcodecinterface_p.h"

#include <stdlib.h>


class QTextCodecFactoryPrivate : public QObject
{
public:
    QTextCodecFactoryPrivate();
    ~QTextCodecFactoryPrivate();

    static QPluginManager<QTextCodecFactoryInterface> *manager;
};


static QTextCodecFactoryPrivate *instance = 0;
QPluginManager<QTextCodecFactoryInterface> *QTextCodecFactoryPrivate::manager = 0;


QTextCodecFactoryPrivate::QTextCodecFactoryPrivate()
    : QObject(qApp)
{
    manager =
	new QPluginManager<QTextCodecFactoryInterface>(IID_QTextCodecFactory,
						   QApplication::libraryPaths(), "/codecs",
						   QLibrary::Delayed, FALSE);

}


QTextCodecFactoryPrivate::~QTextCodecFactoryPrivate()
{
    delete manager;
    manager = 0;
}

#endif // QT_NO_COMPONENT


QTextCodec *QTextCodecFactory::createForName(const QString &name)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    if (! instance)
	instance = new QTextCodecFactoryPrivate;

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    QTextCodecFactoryPrivate::manager->queryInterface(name, &iface );

    if (iface)
	codec = iface->createForName(name);

#endif // QT_NO_COMPONENT

    return codec;
}


QTextCodec *QTextCodecFactory::createForMib(int mib)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    if (! instance)
	instance = new QTextCodecFactoryPrivate;

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    QTextCodecFactoryPrivate::manager->queryInterface("MIB-" + QString::number(mib), &iface );

    if (iface)
	codec = iface->createForMib(mib);

#endif // QT_NO_COMPONENT

    return codec;
}


#endif // QT_NO_TEXTCODEC
