/****************************************************************************
**
** Implementation of QStyleFactory class
**
** Created : 001103
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

#include "qstyleinterface.h" // up here for GCC 2.7.* compatibility
#include "qstylefactory.h"

#include "qinterfacemanager.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#if defined(QT_STYLE_CDE)
#include "qcdestyle.h"
#endif
#if defined(QT_STYLE_MOTIFPLUS)
#include "qmotifplusstyle.h"
#endif
#if defined(QT_STYLE_PLATINUM)
#include "qplatinumstyle.h"
#endif
#if defined(QT_STYLE_SGI)
#include "qsgistyle.h"
#endif
#include <stdlib.h>

static QInterfaceManager<QStyleInterface> *manager = 0;

QStyle *QStyleFactory::create( const QString& style )
{
    if ( !manager )
	manager = new QInterfaceManager<QStyleInterface>( "QStyleInterface", QString((char*)getenv( "QTDIR" )) + "/plugins" );

    QStyleInterface *iface = manager->queryInterface( style );

    if ( iface )
	return iface->create( style );

    if ( style == "Windows" )
	return new QWindowsStyle;
    else if ( style == "Motif" )
	return new QMotifStyle;
#if defined(QT_STYLE_CDE)
    else if ( style == "CDE" )
	return new QCDEStyle;
#endif
#if defined(QT_STYLE_MOTIFPLUS)
    else if ( style == "MotifPlus" )
	return new QMotifPlusStyle;
#endif
#if defined(QT_STYLE_PLATINUM)
    else if ( style == "Platinum" )
	return new QPlatinumStyle;
#endif
#if defined(QT_STYLE_SGI)
    else if ( style == "SGI")
	return new QSGIStyle;
#endif

    return 0;
}

QStringList QStyleFactory::styles()
{
    if ( !manager )
	manager = new QInterfaceManager<QStyleInterface>( "QStyleInterface", QString((char*)getenv( "QTDIR" )) + "/plugins" );

    QStringList list = manager->featureList();

    list << "Windows";
    list << "Motif";
#if defined(QT_STYLE_CDE)
    list << "CDE";
#endif
#if defined(QT_STYLE_MOTIFPLUS)
    list << "MotifPlus";
#endif
#if defined(QT_STYLE_PLATINUM)
    list << "Platinum";
#endif
#if defined(QT_STYLE_SGI)
    list << "SGI";
#endif

    return list;
}
