/****************************************************************************
**
** Implementation of QPluginLoader class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qpluginloader.h"
#include <qfileinfo.h>
#include "qlibrary_p.h"

/*!
    \class QPluginLoader
    \reentrant
    \brief The QPluginLoader class loads a plugin at runtime.

    \mainclass
    \ingroup plugins

    An instance of a QPluginLoader object operates on a single shared
    object file (which we call a "plugin"). A QPluginLoader provides
    access to the functionality in the plugin in a platform
    independent way. You can either pass a file name in the
    constructor, or set it explicitly with setFileName().  If the file
    cannot be found, QPluginLoader tries the name with different
    platform-specific file suffixes, like ".so" on Unix, ".dylib" on
    the Mac, or ".dll" on Windows. This makes it possible to specify
    shared libraries that are only identified by their basename (i.e.
    without their suffix), so the same code will work on different
    operating systems.

    The most important functions are load() to dynamically load the
    plugin file, isLoaded() to check whether loading was successful,
    and instance() to access the root component in the plugin. The
    instance() function implicitly tries to load the plugin if it has
    not been loaded yet. Multiple instances of QPluginLoader can be
    used to access the same physical plugin. Once loaded, plugins
    remain in memory until the application terminates. You can attempt
    to unload a plugin using unload(), but if other instances of
    QPluginLoader are using the same library, the call will fail, and
    unloading will only happen when every instance has called
    unload().

    A plugin is very similar to a shared library, also known as
    "DLL". In fact, technically speaking it is a shared library. Thus
    QPluginLoader is very similar to QLibrary. The main differences
    are that plugins offer safety in cases where a plugin is linked
    against different versions of Qt (something that would result in
    the application crashing when opening such a library with
    QLibrary), and that a plugin offers direct access to a root
    component object - the plugin's instance() - instead of you having
    to resolve a C-function manually.
*/


/*!
    Constructs a plugin loader with the given \a parent.
 */
QPluginLoader::QPluginLoader(QObject *parent)
    :QObject(parent), d(0), did_load(false)
{
}

/*!
    Constructs a plugin loader with the given \a parent that will
    load the plugin specified by \a fileName.

    We recommend omitting the file's suffix in \a fileName, since
    QPluginLoader will automatically look for the file with the appropriate
    suffix in accordance with the platform, e.g. ".so" on Unix,
    ".dylib" on Mac OS X, and ".dll" on Windows. (See \l{fileName}.)

 */
QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
    :QObject(parent), d(0), did_load(false)
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

    The component object is a QObject. Use \l qt_cast to access
    interfaces you are interested in.
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

    Copyright (c) 2002 Jorge Acereda and Peter O'Gorman

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

    We recommend omitting the file's suffix in the file name, since
    QPluginLoader will automatically look for the file with the appropriate
    suffix in accordance with the following list:

    \table
    \header \i Platform \i Supported suffixes
    \row \i Windows     \i dll
    \row \i Unix/Linux  \i so
    \row \i HP-UX       \i sl
    \row \i Mac OS X    \i dylib, bundle, so
    \endtable.

*/
void QPluginLoader::setFileName(const QString &fileName)
{
    if (d) {
        d->release();
        d = 0;
        did_load = false;
    }
    d = QLibraryPrivate::findOrCreate(QFileInfo(fileName).absFilePath());
    if (d && d->pHnd && d->instance)
        did_load = true;
}

QString QPluginLoader::fileName() const
{
    if (d)
        return d->fileName;
    return QString();
}
