/****************************************************************************
**
** Definition of qInitNetworkProtocols function.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QNETWORK_H
#define QNETWORK_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

#ifndef QT_NO_NETWORK

QM_EXPORT_NETWORK void qInitNetworkProtocols();

#endif

#endif
