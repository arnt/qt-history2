/****************************************************************************
**
** Implementation of QFactoryLoader class.
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

#include <qdir.h>
#include <qdebug.h>
#include <qsettings.h>
#include "qmutex.h"
#include "qfactoryinterface.h"
#include "qfactoryloader_p.h"

#include "private/qobject_p.h"

class QFactoryLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFactoryLoader)
public:
    QFactoryLoaderPrivate(){}
    QList<QLibraryPrivate*> libraryList;
    QMap<QString,QLibraryPrivate*> keyMap;
    QStringList keyList;
};

#define d d_func()
#define q q_func()


QFactoryLoader::QFactoryLoader(const char *iid,
                               const QStringList &paths, const QString &suffix,
                               Qt::CaseSensitivity cs, QObject *parent)
    :QObject(*new QFactoryLoaderPrivate, parent)
{
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

    QSettings settings;

    for (int i = 0; i < paths.count(); ++i) {
        QString path = paths.at(i) + suffix;
        if (!QDir(path).exists(QLatin1String("."), true))
            continue;
        QStringList plugins = QDir(path).entryList(filters);
        QLibraryPrivate *library = 0;
        for (int j = 0; j < plugins.count(); ++j) {
            QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));
            library = QLibraryPrivate::findOrCreate(QDir(fileName).canonicalPath());
            if (!library->isPlugin()) {
                library->release();
                continue;
            }
            QString regkey = QString::fromLatin1("/Trolltech/Qt Factory Cache %1.%2/%3:/%4")
                             .arg((QT_VERSION & 0xff0000) >> 16)
                             .arg((QT_VERSION & 0xff00) >> 8)
                             .arg(QLatin1String(iid))
                             .arg(fileName);
            QStringList reg, keys;
            reg = settings.readListEntry(regkey);
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
                settings.writeEntry(regkey, reg);
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
    for (int i = 0; i < d->libraryList.count(); ++i)
        d->libraryList.at(i)->release();
}

QStringList QFactoryLoader::keys() const
{
    return d->keyList;
}

QObject *QFactoryLoader::instance(const QString &key) const
{
    if (QLibraryPrivate* library = d->keyMap.value(key))
        if (library->instance || library->loadPlugin())
            return library->instance();
    return 0;
}

