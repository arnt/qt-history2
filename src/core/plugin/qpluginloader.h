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

#ifndef QPLUGINLOADER_H
#define QPLUGINLOADER_H

#include "qobject.h"


class QLibraryPrivate;

class Q_CORE_EXPORT QPluginLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
public:
    QPluginLoader(QObject *parent = 0);
    QPluginLoader(const QString &fileName, QObject *parent = 0);
    ~QPluginLoader();

    QObject *instance();

    bool load();
    bool unload();
    bool isLoaded() const;

    void setFileName(const QString &fileName);
    QString fileName() const;
private:
    QLibraryPrivate *d;
    bool did_load;
};

#endif //QPLUGINLOADER_H
