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

#include "qstyleinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qstylefactory.h"

#ifndef QT_NO_STYLE

#include "qapplication.h"
#include <private/qpluginmanager_p.h>
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qcdestyle.h"
#include "qmotifplusstyle.h"
#include "qplatinumstyle.h"
#include "qsgistyle.h"
#include "qcompactstyle.h"
#ifndef QT_NO_STYLE_AQUA
#include "qaquastyle.h"
#endif
#if 0
#ifdef Q_WS_MAC
#include "qmacstyle_mac.h"
#endif
#endif
#include <stdlib.h>

#ifndef QT_NO_COMPONENT
class QStyleFactoryPrivate : public QObject
{
public:
    QStyleFactoryPrivate();
    ~QStyleFactoryPrivate();

    static QPluginManager<QStyleFactoryInterface> *manager;
};

static QStyleFactoryPrivate *instance = 0;
QPluginManager<QStyleFactoryInterface> *QStyleFactoryPrivate::manager = 0;

QStyleFactoryPrivate::QStyleFactoryPrivate()
: QObject( qApp )
{
    manager = new QPluginManager<QStyleFactoryInterface>( IID_QStyleFactory, QApplication::libraryPaths(), "/styles", QLibrary::Delayed, FALSE );
}

QStyleFactoryPrivate::~QStyleFactoryPrivate()
{
    delete manager;
    manager = 0;

    instance = 0;
}

#endif //QT_NO_COMPONENT

/*!
  \class QStyleFactory qstylefactory.h
  \brief The QStyleFactory class creates QStyle objects.
  
  The style factory creates a QStyle object for a given key with
  QStyleFactory::create(key).
  
  The styles are either built-in or dynamically loaded from a style
  plugin (see \l QStylePlugin).
  
  QStyleFactory::keys() returns a list of valid keys. Qt currently
  ships with "windows", "motif", "cde", "platinum", "sgi" and
  "motifplus".
  
*/

/*!  Creates a QStyle object that matches \a key. This is either a
  built-in style, or a style from a style plugin.

  \sa keys()
*/
QStyle *QStyleFactory::create( const QString& key )
{
    QString style = key.lower();
#ifndef QT_NO_STYLE_WINDOWS
    if ( style == "windows" )
        return new QWindowsStyle;
    else
#endif

#ifndef QT_NO_STYLE_MOTIF
    if ( style == "motif" )
        return new QMotifStyle;
    else
#endif
#ifndef QT_NO_STYLE_CDE
    if ( style == "cde" )
        return new QCDEStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    if ( style == "motifplus" )
        return new QMotifPlusStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if ( style == "platinum" )
        return new QPlatinumStyle;
    else
#endif
#ifndef QT_NO_STYLE_SGI
    if ( style == "sgi")
        return new QSGIStyle;
    else
#endif
#ifndef QT_NO_STYLE_COMPACT
    if ( style == "compact" )
        return new QCompactStyle;
    else
#endif
#ifndef QT_NO_STYLE_AQUA
    if ( style == "aqua" )
        return new QAquaStyle;
#endif
#if 0
#ifdef Q_WS_MAC
    if( style == "macintosh" )
	return new QMacStyle;
#endif
#endif

#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QStyleFactoryPrivate;

    QInterfacePtr<QStyleFactoryInterface> iface;
    QStyleFactoryPrivate::manager->queryInterface( style, &iface );

    if ( iface )
	return iface->create( style );

#endif
    return 0;
}

#ifndef QT_NO_STRINGLIST
/*!  
  Returns the list of keys  this factory can create
  styles for.

  \sa create()
*/
QStringList QStyleFactory::keys()
{
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QStyleFactoryPrivate;

    QStringList list = QStyleFactoryPrivate::manager->featureList();
#else
    QStringList list;
#endif //QT_NO_COMPONENT

#ifndef QT_NO_STYLE_WINDOWS
    if ( !list.contains( "Windows" ) )
	list << "Windows";
#endif
#ifndef QT_NO_STYLE_MOTIF
    if ( !list.contains( "Motif" ) )
	list << "Motif";
#endif
#ifndef QT_NO_STYLE_CDE
    if ( !list.contains( "CDE" ) )
	list << "CDE";
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    if ( !list.contains( "MotifPlus" ) )
	list << "MotifPlus";
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if ( !list.contains( "Platinum" ) )
	list << "Platinum";
#endif
#ifndef QT_NO_STYLE_SGI
    if ( !list.contains( "SGI" ) )
	list << "SGI";
#endif
#ifndef QT_NO_STYLE_COMPACT
    if ( !list.contains( "Compact" ) )
	list << "Compact";
#endif
#ifndef QT_NO_STYLE_AQUA
    if ( !list.contains( "Aqua" ) )
	list << "Aqua";
#endif
#if 0
#ifdef Q_WS_MAC
    if ( !list.contains( "Macintosh" ) )
	list << "Macintosh";
#endif
#endif

    return list;
}
#endif
#endif // QT_NO_STYLE
