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

#ifndef QFILE_H
#define QFILE_H

#include "qiodevice.h"
#include "qstring.h"

#include <stdio.h>

#ifdef open
#error qfile.h must be included before any system header that defines open
#endif

class QFileEngine;
class QFilePrivate;
class Q_CORE_EXPORT QFile : public QIODevice
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
    Q_DECLARE_PRIVATE(QFile)

public:

    enum Error {
        NoError = 0,
        ReadError = 1,
        WriteError = 2,
        FatalError = 3,
        ResourceError = 4,
        OpenError = 5,
        AbortError = 6,
        TimeOutError = 7,
        UnspecifiedError = 8,
        RemoveError = 9,
        RenameError = 10,
        PositionError = 11,
        ResizeError = 12,
        PermissionsError = 13,
        CopyError = 14
#ifdef QT_COMPAT
        , ConnectError = 30
#endif
    };

    enum Permission {
        ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
        ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
        ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
        ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001
    };
    Q_DECLARE_FLAGS(Permissions, Permission)

    QFile();
    QFile(const QString &name);
#ifndef QT_NO_QOBJECT
    QFile(QObject *parent);
    QFile(const QString &name, QObject *parent);
#endif
    ~QFile();

    Error error() const;
    void unsetError();

    QString fileName() const;
    void setFileName(const QString &name);

    typedef QByteArray (*EncoderFn)(const QString &fileName);
    typedef QString (*DecoderFn)(const QByteArray &localfileName);
    static QByteArray encodeName(const QString &fileName);
    static QString decodeName(const QByteArray &localFileName);
    inline static QString decodeName(const char *localFileName)
        { return decodeName(QByteArray(localFileName)); };
    static void setEncodingFunction(EncoderFn);
    static void setDecodingFunction(DecoderFn);

    bool exists() const;
    static bool exists(const QString &fileName);

    QString readLink() const;
    static QString readLink(const QString &fileName);

    bool remove();
    static bool remove(const QString &fileName);

    bool rename(const QString &newName);
    static bool rename(const QString &oldName, const QString &newName);

    bool link(const QString &newName);
    static bool link(const QString &oldname, const QString &newName);

    bool copy(const QString &newName);
    static bool copy(const QString &fileName, const QString &newName);

    bool isSequential() const;

    bool open(OpenMode flags);
    bool open(OpenMode flags, FILE *);
    bool open(OpenMode flags, int);
    virtual void close();

    Q_LONGLONG size() const;
    Q_LONGLONG pos() const;
    bool seek(Q_LONGLONG offset);
    bool flush();

    bool resize(Q_LONGLONG sz);
    static bool resize(const QString &filename, Q_LONGLONG sz);

    Permissions permissions() const;
    static Permissions permissions(const QString &filename);
    bool setPermissions(Permissions permissionSpec);
    static bool setPermissions(const QString &filename, Permissions permissionSpec);

    int handle() const;

    virtual QFileEngine *fileEngine() const;

#ifdef QT_COMPAT
    typedef Permission PermissionSpec;
    inline QT_COMPAT QString name() const { return fileName(); }
    inline QT_COMPAT void setName(const QString &name) { setFileName(name); }
#endif

protected:
#ifdef QT_NO_QOBJECT
    QFile(QFilePrivate &d);
#else
    QFile(QFilePrivate &d, QObject *parent);
#endif

    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    Q_DISABLE_COPY(QFile)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFile::Permissions)

#endif // QFILE_H
