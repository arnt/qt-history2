/****************************************************************************
** $Id: $
**
** Implementation of QKbdDriverPlugin
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

#include "qkbddriverplugin_qws.h"

#ifndef QT_NO_COMPONENT

#include "qkbddriverinterface_p.h"
#include "qkbd_qws.h"

/*!   \class QKbdDriverPlugin qkbddriverplugin_qws.h
  \brief The QKbdDriverPlugin class provides an abstract base for
  keyboard driver plugins.
  \ingroup plugins

  The keyboard driver plugin is a simple plugin interface that makes it easy
  to create custom keyboard drivers.

  Writing a keyboard driver plugin is achieved by subclassing this
  base class, reimplementing the pure virtual functions keys() and
  create(), and exporting the class with the Q_EXPORT_PLUGIN
  macro.  See the \link plugins-howto.html Plugins
  documentation\endlink for details.
*/

/*! \fn QStringList QKbdDriverPlugin::keys() const

  Returns the list of keyboard drivers this plugin supports.

  \sa create()
*/


/*! \fn  QScreen* QKbdDriverPlugin::create( const QString &driver )

  Creates a driver matching the type specified by \a driver.

  \sa keys()
*/

class QKbdDriverPluginPrivate : public QKbdDriverInterface
{
public:
    QKbdDriverPluginPrivate( QKbdDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QKbdDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

#ifndef QT_NO_STRINGLIST
    QStringList featureList() const;
#endif

    QWSKeyboardHandler* create( const QString& driver );

private:
    QKbdDriverPlugin *plugin;
};

QKbdDriverPluginPrivate::~QKbdDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QKbdDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QKbdDriver )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

#ifndef QT_NO_STRINGLIST
QStringList QKbdDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}
#endif

QWSKeyboardHandler* QKbdDriverPluginPrivate::create( const QString& driver )
{
    qDebug( "Loading plugin: %s", driver.latin1() );
    return plugin->create( driver );
}

/*!
  Constructs a keyboard driver plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/
QKbdDriverPlugin::QKbdDriverPlugin()
    : QGPlugin( d = new QKbdDriverPluginPrivate( this ) )
{
}

/*!
  Destroys the keyboard driver plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.

*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}


/*!\internal
 */
QWSKeyboardHandler* QKbdDriverPlugin::create( const QString& driver )
{
    Q_UNUSED( driver )
    return 0;
}

#endif // QT_NO_COMPONENT
