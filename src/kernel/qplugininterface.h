/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qplugininterface.h#1 $
**
** Definition of QPlugInInterface class
**
** Created : 2000-01-01
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#ifndef QT_H
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

class Q_EXPORT QPlugInInterface
{
public:
    QPlugInInterface() {}
    virtual ~QPlugInInterface() {}

    virtual bool connectNotify( QApplicationInterface* ) { return TRUE; }
    virtual bool disconnectNotify() { return TRUE; }

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QStringList featureList() { return QStringList(); }

    virtual QString queryInterface() const = 0;
};

class Q_EXPORT QPlugInInfo
{
public:
    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QPlugInInterface* queryInterface( const QString& ) = 0;
};

#ifdef _WS_WIN_
#undef QTPLUGINEXPORT
#define QTPLUGINEXPORT __declspec(dllexport)
#else
#define QTPLUGINEXPORT
#endif

#define Q_EXPORT_INTERFACE(INTERFACE, IMPLEMENTATION) \
    extern "C" QTPLUGINEXPORT INTERFACE *qt_load_interface() { return new IMPLEMENTATION(); }

#define Q_EXPORT_PLUGIN( IMPLEMENTATION ) \
    extern "C" QTPLUGINEXPORT QPlugInInfo *qt_load_plugin_info() { return new IMPLEMENTATION(); }

#endif

#endif //QPLUGININTERFACE_H

