/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgplugin.h"

#ifndef QT_NO_COMPONENT

#include <private/qcom_p.h>

QGPlugin::QGPlugin()
    : _iface( 0 )
{
}

QGPlugin::QGPlugin( QUnknownInterface *i )
    : _iface( i )
{
}

QGPlugin::~QGPlugin()
{
}

QUnknownInterface* QGPlugin::iface()
{
    Q_ASSERT( _iface );
    QUnknownInterface *i;
    _iface->queryInterface( IID_QUnknown, &i );
    return i;
}

void QGPlugin::setIface( QUnknownInterface *iface )
{
    _iface = iface;
}

#endif // QT_NO_COMPONENT
