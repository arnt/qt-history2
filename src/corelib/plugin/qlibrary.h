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

#ifndef QLIBRARY_H
#define QLIBRARY_H

#include "QtCore/qobject.h"

class QLibraryPrivate;

class Q_CORE_EXPORT QLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
public:
    explicit QLibrary(QObject *parent = 0);
    explicit QLibrary(const QString& fileName, QObject *parent = 0);
    ~QLibrary();

    void *resolve(const char *symbol);
    static void *resolve(const QString &fileName, const char *symbol);

    bool load();
    bool unload();
    bool isLoaded() const;

    static bool isLibrary(const QString &fileName);

    void setFileName(const QString &fileName);
    QString fileName() const;

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QString library() const { return fileName(); }
    inline QT3_SUPPORT void setAutoUnload( bool ) {}
#endif
private:
    QLibraryPrivate *d;
    bool did_load;
};

#endif //QLIBRARY_H
