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

    ###this class is very similar to QLibrary, except that there are
    no system-specific locations to look for plugins, plus the
    instance() function. Add documention here after QLibrary's
    documentation was reviewed.

*/
QPluginLoader::QPluginLoader(QObject *parent)
    :QObject(parent), d(0), did_load(false)
{
}

QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
    :QObject(parent), d(0), did_load(false)
{
    setFileName(fileName);
}

QPluginLoader::~QPluginLoader()
{
    if (d)
        d->release();
}

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
bool QPluginLoader::unload()
{
    if (did_load) {
        did_load = false;
        return d->unload();
    }
    return false;
}

bool QPluginLoader::isLoaded() const
{
    return d && d->pHnd && d->instance;
}

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
