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
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_LIBRARY

class QLibraryPrivate;

class Q_CORE_EXPORT QLibrary
{
public:
    QLibrary(const QString& filename);
    virtual ~QLibrary();

    void *resolve(const char*);
    static void *resolve(const QString &filename, const char *);

    bool load();
    virtual bool unload();
    bool isLoaded() const;

    bool autoUnload() const;
    void setAutoUnload(bool enable);

    QString library() const;

private:
    QLibraryPrivate *d;

    QString libfile;
    uint aunload : 1;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLibrary(const QLibrary &);
    QLibrary &operator=(const QLibrary &);
#endif
};

#endif //QT_NO_LIBRARY
#endif //QLIBRARY_H
