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

#include "qplatformdefs.h"

#include "qplugin.h"
#include "qpluginloader.h"
#include <qfileinfo.h>
#include "qlibrary_p.h"
#include "qdebug.h"

#ifndef QT_NO_LIBRARY

/*!
    \class QPluginLoader
    \reentrant
    \brief The QPluginLoader class loads a plugin at run-time.

    \mainclass
    \ingroup plugins

    QPluginLoader provides access to a \l{How to Create Qt
    Plugins}{Qt plugin}. A Qt plugin is stored in a shared library (a
    DLL) and offers these benefits over shared libraries accessed
    using QLibrary:

    \list
    \o QPluginLoader checks that a plugin is linked against the same
       version of Qt as the application.
    \o QPluginLoader provides direct access to a root component object
       (instance()), instead of forcing you to resolve a C function manually.
    \endlist

    An instance of a QPluginLoader object operates on a single shared
    library file, which we call a plugin. It provides access to the
    functionality in the plugin in a platform-independent way. To
    specify which plugin to load, either pass a file name in
    the constructor or set it with setFileName().

    The most important functions are load() to dynamically load the
    plugin file, isLoaded() to check whether loading was successful,
    and instance() to access the root component in the plugin. The
    instance() function implicitly tries to load the plugin if it has
    not been loaded yet. Multiple instances of QPluginLoader can be
    used to access the same physical plugin.

    Once loaded, plugins remain in memory until the application
    terminates. You can attempt to unload a plugin using unload(),
    but if other instances of QPluginLoader are using the same
    library, the call will fail, and unloading will only happen when
    every instance has called unload().

    In order to speed up loading and validation of plugins, some of
    the information that is collected during loading is cached in
    persistent memory (through QSettings). For instance, the result
    of a load operation (e.g. succeeded or failed) is stored in the
    cache, so that subsequent load operations don't try to load an
    invalid plugin. However, if the "last modified" timestamp of
    a plugin has changed, the plugin's cache entry is invalidated
    and the plugin is reloaded regardless of the values in the cache
    entry. The cache entry is then updated with the new result of the
    load operation.

    This also means that the timestamp must be updated each time the
    plugin or any dependent resources (such as a shared library) is
    updated, since the dependent resources might influence the result
    of loading a plugin.

    See \l{How to Create Qt Plugins} for more information about
    how to make your application extensible through plugins.

    \sa QLibrary, {Plug & Paint Example}
*/

/*!
    Constructs a plugin loader with the given \a parent.
*/
QPluginLoader::QPluginLoader(QObject *parent)
    : QObject(parent), d(0), did_load(false)
{
}

/*!
    Constructs a plugin loader with the given \a parent that will
    load the plugin specified by \a fileName.

    To be loadable, the file's suffix must be a valid suffix for a
    loadable library in accordance with the platform, e.g. \c .so on
    Unix, - \c .dylib on Mac OS X, and \c .dll on Windows. The suffix
    can be verified with QLibrary::isLibrary().

    \sa setFileName()
*/
QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
    : QObject(parent), d(0), did_load(false)
{
    setFileName(fileName);
}

/*!
    Destroys the QPluginLoader object.

    Unless unload() was called explicitly, the plugin stays in memory
    until the application terminates.

    \sa isLoaded(), unload()
*/
QPluginLoader::~QPluginLoader()
{
    if (d)
        d->release();
}

/*!
    Returns the root component object of the plugin. The plugin is
    loaded if necessary. The function returns 0 if the plugin could
    not be loaded or if the root component object could not be
    instantiated.

    If the root component object was destroyed, calling this function
    creates a new instance.

    The root component, returned by this function, is not deleted when
    the QPluginLoader is destroyed.

    The component object is a QObject. Use qobject_cast() to access
    interfaces you are interested in.

    \sa load()
*/
QObject *QPluginLoader::instance()
{
    if (!d)
        return 0;
    if (!d->pHnd)
        load();
    if (d->instance)
        return d->instance();
    return 0;
}

/*!
    Loads the plugin and returns true if the plugin was loaded
    successfully; otherwise returns false. Since instance() always
    calls this function before resolving any symbols it is not
    necessary to call it explicitly. In some situations you might want
    the plugin loaded in advance, in which case you would use this
    function.

    \sa unload()
*/
bool QPluginLoader::load()
{
    if (!d)
        return false;
    if (did_load)
        return d->pHnd && d->instance;
    if (!d->isPlugin())
        return false;
    did_load = true;
    return d->loadPlugin();
}


/*!
    Unloads the plugin and returns true if the plugin could be
    unloaded; otherwise returns false.

    This happens automatically on application termination, so you
    shouldn't normally need to call this function.

    If other instances of QPluginLoader are using the same plugin, the
    call will fail, and unloading will only happen when every instance
    has called unload().

    \warning The root component of a plugin, returned by the instance()
    function, becomes invalid once the plugin is unloaded. Delete the
    root component before unloading the plugin. Attempting to access
    members of invalid root components will in most cases result in a
    segmentation fault.

    \sa instance(), load()
*/
bool QPluginLoader::unload()
{
    if (did_load) {
        did_load = false;
        return d->unload();
    }
    if (d)  // Ouch
        d->errorString = tr("The plugin was not loaded.");
    return false;
}

/*!
    Returns true if the plugin is loaded; otherwise returns false.

    \sa load()
 */
bool QPluginLoader::isLoaded() const
{
    return d && d->pHnd && d->instance;
}

/*!
    \property QPluginLoader::fileName
    \brief the file name of the plugin

    To be loadable, the file's suffix must be a valid suffix for a
    loadable library in accordance with the platform, e.g. \c .so on
    Unix, \c .dylib on Mac OS X, and \c .dll on Windows. The suffix
    can be verified with QLibrary::isLibrary().

    \sa load()
*/
void QPluginLoader::setFileName(const QString &fileName)
{
#if defined(QT_SHARED)
    if (d) {
        d->release();
        d = 0;
        did_load = false;
    }
    d = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());
#else
    if (qt_debug_component()) {
        qWarning("Cannot load %s into a statically linked Qt library.",
            (const char*)QFile::encodeName(fileName));
    }
    Q_UNUSED(fileName);
#endif
}

QString QPluginLoader::fileName() const
{
    if (d)
        return d->fileName;
    return QString();
}

/*!
    \since 4.2

    Returns a text string with the description of the last error that occurred.
*/
QString QPluginLoader::errorString() const
{
    return !d || d->errorString.isEmpty() ? tr("Unknown error") : d->errorString;
}

typedef QList<QtPluginInstanceFunction> StaticInstanceFunctionList;
Q_GLOBAL_STATIC(StaticInstanceFunctionList, staticInstanceFunctionList)

/*! \since 4.4

    \property QPluginLoader::loadHints
    \brief Give the load() function some hints on how it should behave.

    You can give hints on how the symbols in the plugin are
    resolved. By default, none of the hints are set.

    See the documentation of QLibrary::loadHints for a complete
    description of how this property works.

    \sa QLibrary::loadHints
*/

void QPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
    d->loadHints = loadHints;
}

QLibrary::LoadHints QPluginLoader::loadHints() const
{
    return d->loadHints;
}

/*!
    \relates QPluginLoader

    Registers the given \a function with the plugin loader.
*/
void Q_CORE_EXPORT qRegisterStaticPluginInstanceFunction(QtPluginInstanceFunction function)
{
    staticInstanceFunctionList()->append(function);
}

/*!
    Returns a list of static plugin instances (root components) held
    by the plugin loader.
*/
QObjectList QPluginLoader::staticInstances()
{
    QObjectList instances;
    StaticInstanceFunctionList *functions = staticInstanceFunctionList();
    if (functions) {
        for (int i = 0; i < functions->count(); ++i)
            instances.append((*functions)[i]());
    }
    return instances;
}
#endif // QT_NO_LIBRARY
