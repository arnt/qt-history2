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

#include "qfactoryloader_p.h"
#include "qfactoryinterface.h"
#include "qmap.h"
#include <qdir.h>
#include <qsettings.h>
#include <qdebug.h>
#include "qplugin.h"
#include "qpluginloader.h"
#include "private/qobject_p.h"

class QFactoryLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFactoryLoader)
public:
    QFactoryLoaderPrivate(){}
    QByteArray iid;
    QList<QLibraryPrivate*> libraryList;
    QMap<QString,QLibraryPrivate*> keyMap;
    QStringList keyList;
};

QFactoryLoader::QFactoryLoader(const char *iid,
                               const QStringList &paths, const QString &suffix,
                               Qt::CaseSensitivity cs, QObject *parent)
    :QObject(*new QFactoryLoaderPrivate, parent)
{
    Q_D(QFactoryLoader);
    d->iid = iid;
    QStringList filters;
#if defined(Q_OS_WIN32)
    filters << QLatin1String("*.dll");
#elif defined(Q_OS_DARWIN)
    filters << QLatin1String("*.dylib") << QLatin1String("*.so") << QLatin1String("*.bundle");
#elif defined(Q_OS_HPUX)
    filters << QLatin1String("*.sl");
#elif defined(Q_OS_UNIX)
    filters << QLatin1String("*.so");
#endif

    QSettings settings(Qt::UserScope, QLatin1String("trolltech.com"));

    for (int i = 0; i < paths.count(); ++i) {
        QString path = paths.at(i) + suffix;
        if (!QDir(path).exists(QLatin1String(".")))
            continue;
        QStringList plugins = QDir(path).entryList(filters);
        QLibraryPrivate *library = 0;
        for (int j = 0; j < plugins.count(); ++j) {
            QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));
            library = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());
            if (!library->isPlugin()) {
                library->release();
                continue;
            }
            QString regkey = QString::fromLatin1("Qt Factory Cache %1.%2/%3:/%4")
                             .arg((QT_VERSION & 0xff0000) >> 16)
                             .arg((QT_VERSION & 0xff00) >> 8)
                             .arg(QLatin1String(iid))
                             .arg(fileName);
            QStringList reg, keys;
            reg = settings.value(regkey).toStringList();
            if (reg.count() && library->lastModified == reg[0]) {
                keys = reg;
                keys.removeFirst();
            } else {
                if (!library->loadPlugin()) {
                    library->release();
                    continue;
                }
                QObject *instance = library->instance();
                QFactoryInterface *factory = qt_cast<QFactoryInterface*>(instance);
                if (instance && factory && instance->qt_metacast(iid))
                    keys = factory->keys();
                if (keys.isEmpty())
                    library->unload();
                reg.clear();
                reg << library->lastModified;
                reg += keys;
                settings.setValue(regkey, reg);
            }
            if (!keys.isEmpty()) {
                d->libraryList += library;
                for (int k = 0; k < keys.count(); ++k) {
                    // first come first serve, unless the first
                    // library was built with a future Qt version,
                    // whereas the new one has a Qt version that fits
                    // better
                    QString key = keys.at(k);
                    if (!cs)
                        key = key.toLower();
                    QLibraryPrivate *previous = d->keyMap.value(key);
                    if (!previous || (previous->qt_version > QT_VERSION && library->qt_version <= QT_VERSION)) {
                        d->keyMap[key] = library;
                        d->keyList += keys.at(k);
                    }
                }
            } else {
                qWarning("In %s:\n Plugin does not implement factory interface %s", QFile::encodeName(fileName).constData(), iid);
                library->release();
            }
        }
    }
}

QFactoryLoader::~QFactoryLoader()
{
    Q_D(QFactoryLoader);
    for (int i = 0; i < d->libraryList.count(); ++i)
        d->libraryList.at(i)->release();
}

QStringList QFactoryLoader::keys() const
{
    Q_D(const QFactoryLoader);
    QStringList keys = d->keyList;
    QObjectList instances = QPluginLoader::staticInstances();
    for (int i = 0; i < instances.count(); ++i)
        if (QFactoryInterface *factory = qt_cast<QFactoryInterface*>(instances.at(i)))
            if (instances.at(i)->qt_metacast(d->iid))
                    keys += factory->keys();
    return keys;
}

QObject *QFactoryLoader::instance(const QString &key) const
{
    Q_D(const QFactoryLoader);
    QObjectList instances = QPluginLoader::staticInstances();
    for (int i = 0; i < instances.count(); ++i)
        if (QFactoryInterface *factory = qt_cast<QFactoryInterface*>(instances.at(i)))
            if (instances.at(i)->qt_metacast(d->iid) && factory->keys().contains(key, Qt::CaseInsensitive))
                return instances.at(i);

    if (QLibraryPrivate* library = d->keyMap.value(key)) {
        if (library->instance || library->loadPlugin()) {
            if (QObject *obj = library->instance()) {
                if (obj && !obj->parent())
                    obj->setParent(const_cast<QFactoryLoader*>(this));
                return obj;
            }
        }
    }
    return 0;
}
