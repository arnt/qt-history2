/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qscriptextensionplugin.h"

#ifndef QT_NO_SCRIPT

#include "qscriptvalue.h"
#include "qscriptengine.h"

/*!
    \since 4.3
    \class QScriptExtensionPlugin
    \brief The QScriptExtensionPlugin class provides an abstract base for custom QScript extension plugins.

    \ingroup plugins

    QScriptExtensionPlugin is a plugin interface that makes it
    possible to offer extensions that can be loaded dynamically into
    applications using the QScriptEngine class.

    Writing a script extension plugin is achieved by subclassing this
    base class, reimplementing the pure virtual keys() and initialize()
    functions, and exporting the class using the Q_EXPORT_PLUGIN2()
    macro. See \l {How to Create Qt Plugins} for details.

    \sa QScriptEngine::importExtension(), {Creating QtScript Extensions}
*/

/*!
    \fn QStringList QScriptExtensionPlugin::keys() const

    Returns the list of keys this plugin supports.

    These keys are usually the names of the "modules" or "packages"
    that are implemented in the plugin (e.g. \c{com.mycompany.MyProduct}).

    \sa initialize()
*/

/*!
    \fn void QScriptExtensionPlugin::initialize(const QString& key, QScriptEngine *engine)

    Initializes the extension specified by \a key in the given \a engine.
    The key must come from the set of keys().

    \sa keys()
*/

/*!
    Constructs a script extension plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QScriptExtensionPlugin::QScriptExtensionPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the script extension plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QScriptExtensionPlugin::~QScriptExtensionPlugin()
{
}

/*!

  This function is provided for convenience when reimplementing
  initialize(). It splits the given \a key on \c{'.'} (dot), and
  ensures that there's a corresponding path of objects in the
  environment of the given \a engine, creating new objects to complete
  the path if necessary.  E.g. if the key is "com.trolltech", after
  the call to setupPackage() the script expression \c{com.trolltech}
  will evaluate to an object. More specifically, the engine's Global
  Object will have a property called "com", which in turn has a
  property called "trolltech".

  Use this function to avoid global namespace pollution when installing
  your extensions in the engine.

  \sa initialize()
*/
QScriptValue QScriptExtensionPlugin::setupPackage(
    const QString &key, QScriptEngine *engine) const
{
    QStringList components = key.split(QLatin1Char('.'));
    QScriptValue o = engine->globalObject();
    for (int i = 0; i < components.count(); ++i) {
        QScriptValue oo = o.property(components.at(i));
        if (!oo.isValid()) {
            oo = engine->newObject();
            o.setProperty(components.at(i), oo);
        }
        o = oo;
    }
    return o;
}

#endif // QT_NO_SCRIPT
