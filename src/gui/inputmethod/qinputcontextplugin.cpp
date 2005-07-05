/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** Implementation of QInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#include "qinputcontextplugin.h"

#ifndef QT_NO_IM
#ifndef QT_NO_LIBRARY

/*!
    \class QInputContextPlugin
    \brief The QInputContextPlugin class provides an abstract base for custom QInputContext plugins.
    \reentrant
    \ingroup plugins

    The input context plugin is a simple plugin interface that makes it
    easy to create custom input contexts that can be loaded dynamically
    into applications.

    Writing a input context plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys(),
    create(), languages(), displayName(), and description(), and
    exporting the class with the Q_EXPORT_PLUGIN() macro.

    \sa QInputContext, {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QInputContextPlugin::keys() const

    Returns the list of QInputContext keys this plugin provides.

    These keys are usually the class names of the custom input context
    that are implemented in the plugin.

    Return value is the names to identify and specify input methods
    for the input method switching mechanism and so on. The names have
    to be consistent with QInputContext::identifierName(). The names
    have to consist of ASCII characters only. See also
    QInputContext::identifierName() for further information.

    \sa create(), displayName(), QInputContext::identifierName()
*/

/*!
    \fn QInputContext* QInputContextPlugin::create( const QString& key )

    Creates and returns a QInputContext instance for the input context key \a key.
    The input context key is usually the class name of the required input method.

    \sa keys()
*/

/*!
    \fn QStringList QInputContextPlugin::languages(const QString &key)

    Returns what languages are supported by the QInputContext instance
    specified by \a key.

    The languages are expressed as language code (e.g. "zh_CN",
    "zh_TW", "zh_HK", "ja", "ko", ...). An input context that suports
    multiple languages can return all supported languages as
    QStringList. The name has to be consistent with
    QInputContextPlugin::language().

    This information may be used to optimize user interface.

    \sa QInputContext::language()
*/

/*!
    \fn QString QInputContextPlugin::displayName(const QString &key)

    Returns a user friendly i18n-ized name of the QInputContext
    instance specified by \a key. This string may be appeared in a
    menu and so on for users.

    There are two different names with different responsibility in the
    input method domain. This function returns one of them. Another
    name is called 'identifier name' to identify and specify input
    methods for the input method switching mechanism and so on.

    Although tr( identifierName ) can provide user friendly i18n-ized
    name without this function, the message catalog have to be managed
    by Qt in the case. However, some sophisticated input method
    framework manages their own message catalogs to provide this
    i18n-ized name string. So we need this function rather than just
    call tr() for identifier name.

    \sa keys(), QInputContext::identifierName()
*/

/*!
    \fn QString QInputContextPlugin::description(const QString &key)

    Returns a i18n-ized brief description of the QInputContext
    instance specified by \a key. This string may be appeared in some
    user interfaces.
*/


/*!
    Constructs a input context plugin with the given \a parent. This
    is invoked automatically by the Q_EXPORT_PLUGIN() macro.
*/
QInputContextPlugin::QInputContextPlugin(QObject *parent)
    :QObject(parent)
{
}

/*!
    Destroys the input context plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QInputContextPlugin::~QInputContextPlugin()
{
}

#endif // QT_NO_LIBRARY
#endif // QT_NO_IM
