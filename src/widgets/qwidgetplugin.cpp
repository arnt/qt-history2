/****************************************************************************
** $Id: $
**
** Implementation of QWidgetPlugin class
**
** Created : 010920
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qwidgetplugin.h"

#ifndef QT_NO_WIDGETPLUGIN
#include "qwidgetinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qwidget.h"

/*!   \class QWidgetPlugin qwidgetplugin.h
  \brief The QWidgetPlugin class provides an abstract base for custom QWidget plugins.
  \ingroup plugins

  The widget plugin is a simple plugin interface that makes it easy to
  create custom widgets that can be included in forms using \link
  designer-manual.book Qt Designer\endlink and used by applications.

  Writing a widget plugin is achieved by subclassing this base class,
  reimplementing the pure virtual functions keys(), create(), group(),
  iconSet(), includeFile(), toolTip(), whatsThis() and isContainer(),
  and exporting the class with the Q_EXPORT_PLUGIN macro.

    See the \link designer-manual.book Qt Designer manual's\endlink,
    'Creating Custom Widgets' section in the 'Creating Custom Widgets'
    chapter, for a complete example of a QWidgetPlugin.

  See also the \link plugins-howto.html Plugins documentation\endlink.
*/

class QWidgetPluginPrivate : public QWidgetFactoryInterface, QLibraryInterface
{
public:
    QWidgetPluginPrivate( QWidgetPlugin *p )
	: plugin( p )
    {
    }

    virtual ~QWidgetPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QWidget *create( const QString &key, QWidget *parent, const char *name );
    QString group( const QString &widget ) const;
    QIconSet iconSet( const QString &widget ) const;
    QString includeFile( const QString &widget ) const;
    QString toolTip( const QString &widget ) const;
    QString whatsThis( const QString &widget ) const;
    bool isContainer( const QString &widget ) const;

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QWidgetPlugin *plugin;
    QObjectCleanupHandler widgets;
};

QRESULT QWidgetPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QWidgetFactory )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

/*! \fn QStringList QWidgetPlugin::keys() const

  Returns the list of widget keys this plugin supports.

  These keys must be the class names of the custom widgets that are
  implemented in the plugin.

  \sa create()
*/

/*! \fn QWidget *QWidgetPlugin::create( const QString &, QWidget *, const char * )

  Creates and returns a QWidget object for the widget key \a key. The
  widget key is the class name of the required widget. The
  \a name and \a parent arguments are passed to the custom widget's
  constructor.

  \sa keys()
*/

QWidgetPluginPrivate::~QWidgetPluginPrivate()
{
    delete plugin;
}

QStringList QWidgetPluginPrivate::featureList() const
{
    return plugin->keys();
}

QWidget *QWidgetPluginPrivate::create( const QString &key, QWidget *parent, const char *name )
{
    QWidget *w = plugin->create( key, parent, name );
    widgets.add( w );
    return w;
}

QString QWidgetPluginPrivate::group( const QString &widget ) const
{
    return plugin->group( widget );
}

QIconSet QWidgetPluginPrivate::iconSet( const QString &widget ) const
{
    return plugin->iconSet( widget );
}

QString QWidgetPluginPrivate::includeFile( const QString &widget ) const
{
    return plugin->includeFile( widget );
}

QString QWidgetPluginPrivate::toolTip( const QString &widget ) const
{
    return plugin->toolTip( widget );
}

QString QWidgetPluginPrivate::whatsThis( const QString &widget ) const
{
    return plugin->whatsThis( widget );
}

bool QWidgetPluginPrivate::isContainer( const QString &widget ) const
{
    return plugin->isContainer( widget );
}

bool QWidgetPluginPrivate::init()
{
    return TRUE;
}

void QWidgetPluginPrivate::cleanup()
{
    widgets.clear();
}

bool QWidgetPluginPrivate::canUnload() const
{
    return widgets.isEmpty();
}


/*!
  Constructs a widget plugin. This is invoked automatically by
  the Q_EXPORT_PLUGIN macro.
*/
QWidgetPlugin::QWidgetPlugin()
    : QGPlugin( (QWidgetFactoryInterface*)(d = new QWidgetPluginPrivate( this )) )
{
}

/*!
  Destroys the widget plugin.

  You never have to call this explicitly. Qt destroys a plugin
  automatically when it is no longer used.

*/
QWidgetPlugin::~QWidgetPlugin()
{
    // don't delete d, as this is deleted by d
}

/*!
    Returns the group (toolbar name) that the custom widget of class \a
    key should be part of when \e{Qt Designer} loads it.

    The default implementation returns a null string.
*/
QString QWidgetPlugin::group( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the iconset that \e{Qt Designer} should use to represent
    the custom widget of class \a key in the toolbar.

    The default implementation returns an null iconset.
*/
QIconSet QWidgetPlugin::iconSet( const QString & ) const
{
    return QIconSet();
}

/*!
    Returns the name of the include file that \e{Qt Designer} and \c
    uic should use to include the custom widget of class \a key in
    generated code.

    The default implementation returns a null string.
*/
QString QWidgetPlugin::includeFile( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the text of the tooltip that \e{Qt Designer} should use
    for the custom widget of class \a key's toolbar button.

    The default implementation returns a null string.
*/
QString QWidgetPlugin::toolTip( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the text of the whatsThis text that \e{Qt Designer} should
    use when the user requests whatsThis help for the custom widget
    of class \a key.

    The default implementation returns a null string.
*/
QString QWidgetPlugin::whatsThis( const QString & ) const
{
    return QString::null;
}

/*!
    Returns TRUE if the custom widget of class \a key can contain other
    widgets, e.g. like QFrame; otherwise returns FALSE.

    The default implementation returns FALSE.
*/
bool QWidgetPlugin::isContainer( const QString & ) const
{
    return FALSE;
}

#endif //QT_NO_WIDGETPLUGIN
