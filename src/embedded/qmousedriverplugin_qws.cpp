/****************************************************************************
** $Id: $
**
** Implementation of QMouseDriverPlugin
**
** Created : 20020218
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmousedriverplugin_qws.h"

#ifndef QT_NO_COMPONENT

#include "qmousedriverinterface_p.h"
#include "qmouse_qws.h"

/*!   \class QMouseDriverPlugin qmousedriverplugin_qws.h
  \brief The QMouseDriverPlugin class provides an abstract base for
  mouse driver plugins.
  \ingroup plugins

  The mouse driver plugin is a simple plugin interface that makes it easy
  to create custom mouse drivers.

  Writing a mouse driver plugin is achieved by subclassing this
  base class, reimplementing the pure virtual functions keys() and
  create(), and exporting the class with the Q_EXPORT_PLUGIN
  macro.  See the \link plugins-howto.html Plugins
  documentation\endlink for details.
*/

/*! \fn QStringList QMouseDriverPlugin::keys() const

  Returns the list of mouse drivers this plugin supports.

  \sa create()
*/


/*! \fn  QScreen* QMouseDriverPlugin::create( const QString &driver )

  Creates a driver matching the type specified by \a driver.

  \sa keys()
*/

class QMouseDriverPluginPrivate : public QMouseDriverInterface
{
public:
    QMouseDriverPluginPrivate( QMouseDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QMouseDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

#ifndef QT_NO_STRINGLIST
    QStringList featureList() const;
#endif

    QWSMouseHandler* create( const QString& driver, const QString &device );

private:
    QMouseDriverPlugin *plugin;
};

QMouseDriverPluginPrivate::~QMouseDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QMouseDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QMouseDriver )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

#ifndef QT_NO_STRINGLIST
QStringList QMouseDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}
#endif

QWSMouseHandler* QMouseDriverPluginPrivate::create( const QString& driver, const QString &device )
{
    qDebug( "Loading plugin: %s", driver.latin1() );
    return plugin->create( driver, device );
}

/*!
  Constructs a mouse driver plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/
QMouseDriverPlugin::QMouseDriverPlugin()
    : QGPlugin( d = new QMouseDriverPluginPrivate( this ) )
{
}

/*!
  Destroys the mouse driver plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.

*/
QMouseDriverPlugin::~QMouseDriverPlugin()
{
}


/*!\internal
 */
QWSMouseHandler* QMouseDriverPlugin::create( const QString& driver, const QString &device )
{
    Q_UNUSED( driver )
    Q_UNUSED( device )
    return 0;
}

#endif // QT_NO_COMPONENT
