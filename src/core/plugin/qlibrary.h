/****************************************************************************
**
** Definition of QLibrary class.
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

#ifndef QLIBRARY_H
#define QLIBRARY_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QLibraryPrivate;

class Q_CORE_EXPORT QLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
public:
    QLibrary(QObject *parent = 0);
    QLibrary(const QString& fileName, QObject *parent = 0);
    ~QLibrary();

    void *resolve(const char *symbol);
    static void *resolve(const QString &fileName, const char *symbol);

    bool load();
    bool unload();
    bool isLoaded() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

#ifdef QT_COMPAT
    inline QT_COMPAT QString library() const { return fileName(); }
    inline QT_COMPAT void setAutoUnload( bool ) {}
#endif
private:
    QLibraryPrivate *d;
    bool did_load;
};


#endif //QLIBRARY_H
