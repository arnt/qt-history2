/****************************************************************************
**
** Implementation of QAccessiblePlugin class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessibleplugin.h"

#ifndef QT_NO_ACCESSIBLEPLUGIN

#include "qaccessible.h"

/*!
    \class QAccessiblePlugin qaccessibleplugin.h
    \brief The QAccessiblePlugin class provides an abstract base for 
    accessibility plugins.

    \ingroup plugins
*/

class QAccessiblePluginPrivate : public QAccessibleFactoryInterface
{
public:
    QAccessiblePluginPrivate(QAccessiblePlugin *p)
	: plugin(p)
    {
    }

    virtual ~QAccessiblePluginPrivate();

    QRESULT queryInterface(const QUuid &iid, QUnknownInterface **iface);
    Q_REFCOUNT;

    QStringList featureList() const;
    QRESULT createAccessibleInterface(const QString &, QObject *, QAccessibleInterface**);

private:
    QAccessiblePlugin *plugin;
};

QAccessiblePluginPrivate::~QAccessiblePluginPrivate()
{
    delete plugin;
}

QRESULT QAccessiblePluginPrivate::queryInterface(const QUuid &iid, QUnknownInterface **iface)
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QAccessibleFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QAccessibleFactory )
	*iface = (QAccessibleFactoryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}


QStringList QAccessiblePluginPrivate::featureList() const
{
    return plugin->keys();
}

QRESULT QAccessiblePluginPrivate::createAccessibleInterface(const QString &key, 
    QObject *object, QAccessibleInterface **iface)
{
    *iface = 0;
    QAccessibleInterface *aif = plugin->create(key, object);
    if (!aif)
	return QE_NOINTERFACE;

    *iface = aif;
    aif->addRef();
    return QS_OK;
}

/*!
    Constructs an accessibility plugin. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN macro.
*/
QAccessiblePlugin::QAccessiblePlugin()
: QGPlugin((QAccessibleFactoryInterface*)(d = new QAccessiblePluginPrivate(this)))
{
}

/*!
    Destroys the widget plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QAccessiblePlugin::~QAccessiblePlugin()
{
    // don't delete d, as this is deleted by d
}

/*!
    \fn QStringList QAccessiblePlugin::keys() const

    Returns the list of keys this plugin supports.

    These keys must be the class names that this plugin provides 
    an accessibility implementation for.

    \sa create()
*/

/*!
    \fn QAccessibleInterface *QAccessiblePlugin::create(const QString &key, QObject *object)

    Creates and returns a QAccessibleInterface implementation for the 
    class \a key and the object \a object.

    \sa keys()
*/

#endif // QT_NO_ACCESSIBLEPLUGIN
