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

#include "qplatformdefs.h"

#include "qplugin.h"
#include "qpluginloader.h"
#include <qfileinfo.h>
#include "qlibrary_p.h"
#include "qdebug.h"

/*!
    \class QPluginLoader
    \reentrant
    \brief The QPluginLoader class loads a plugin at run-time.

    \mainclass
    \ingroup plugins

    QPlugingLoader provides a means for accessing a \l{How to Create
    Qt Plugins}{Qt plugin}. Qt plugins are stored in a shared library
    (a "DLL") and follow offer a few benefits over plain shared
    libraries accessed using QLibrary:

    \list
    \o Plugins offer safety in cases where a plugin is linked against
       different versions of Qt. (This typically would result in the
       application crashing when opening such a library with
       QLibrary.)
    \o Plugin offers direct access to a root component object
       (instance()), instead of forcing you to resolve a C function
       manually.
    \endlist

    An instance of a QPluginLoader object operates on a single shared
    library file, which we call a plugin. It provides access to the
    functionality in the plugin in a platform-independent way. To
    specify which plugin to load, you can either pass a file name in
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

    See \l{How to Create Qt Plugins} for more information about
    how to make your application extensible through plugins.

    \sa QLibrary
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

    The instance is not deleted when the QPluginLoader is destroyed.

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

    On Darwin and Mac OS X this function uses code from dlcompat, part of the
    OpenDarwin project.

    \sa unload()

    \legalese
    \code

    Copyright (c) 2002 Jorge Acereda and Peter O'Gorman.

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    \endcode
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

    \sa instance(), load()
*/
bool QPluginLoader::unload()
{
    if (did_load) {
        did_load = false;
        return d->unload();
    }
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
    Unix, - \c .dylib on Mac OS X, and \c .dll on Windows. The suffix
    can be verified with QLibrary::isLibrary().

    \sa load()
*/
void QPluginLoader::setFileName(const QString &fileName)
{
    if (d) {
        d->release();
        d = 0;
        did_load = false;
    }
    d = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());
    if (d && d->pHnd && d->instance)
        did_load = true;
}

QString QPluginLoader::fileName() const
{
    if (d)
        return d->fileName;
    return QString();
}
typedef QList<QtPluginInstanceFunction> StaticInstanceFunctionList;
Q_GLOBAL_STATIC(StaticInstanceFunctionList, staticInstanceFunctionList)

void Q_CORE_EXPORT qRegisterStaticPluginInstanceFunction(QtPluginInstanceFunction function)
{
    staticInstanceFunctionList()->append(function);
}

/*!

*/
QObjectList QPluginLoader::staticInstances()
{
    QObjectList instances;
    StaticInstanceFunctionList *functions = staticInstanceFunctionList();
    for (int i = 0; i < functions->count(); ++i)
        instances.append((*functions)[i]());
    return instances;
}
