/****************************************************************************
**
** Implementation of QAccessiblePlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessibleplugin.h"

#ifndef QT_NO_ACCESSIBILITY

#include "qaccessible.h"

/*!
    \class QAccessiblePlugin qaccessibleplugin.h
    \brief The QAccessiblePlugin class provides an abstract base for
    accessibility plugins.

    \ingroup plugins
    \ingroup accessibility
*/

/*!
    Constructs an accessibility plugin. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN macro.
*/
QAccessiblePlugin::QAccessiblePlugin(QObject *parent)
: QObject(parent)
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

#endif // QT_NO_ACCESSIBILITY
