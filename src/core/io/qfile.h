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

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QFilePrivate;
class Q_CORE_EXPORT QFile : public QIODevice
{
    Q_DECLARE_PRIVATE(QFile)

public:
    enum PermissionSpec {
        ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
        ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
        ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
        ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001 
    };

    QFile();
    QFile(const QString &name);
    ~QFile();

    QString fileName() const;
    void setFileName(const QString &name);
#ifdef QT_COMPAT
    inline QT_COMPAT QString name() const { return fileName(); }
    inline QT_COMPAT void setName(const QString &name) { setFileName(name); }
#endif

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

    bool remove();
    static bool remove(const QString &fileName);

    bool rename(const QString &newName);
    static bool rename(const QString &oldName, const QString &newName);

    bool copy(const QString &newName);
    static bool copy(const QString &fileName, const QString &newName);

#ifdef Q_NO_USING_KEYWORD
    inline bool open(int mode) { return QIODevice::open(mode); }
#else
    using QIODevice::open;
#endif
    bool open(int, FILE *);
    bool open(int, int);

#ifdef Q_NO_USING_KEYWORD
    inline Q_LONG readLine(char *data, Q_LONG maxlen) { return QIODevice::readLine(data, maxlen); }
#else
    using QIODevice::readLine;
#endif
    Q_LONG readLine(QString &string, Q_LONG maxlen);

    bool resize(QIODevice::Offset sz);
    static bool resize(const QString &filename, QIODevice::Offset sz);

    uint permissions() const;
    static uint permissions(const QString &filename);
    bool setPermissions(uint permissionSpec);
    static bool setPermissions(const QString &filename, uint permissionSpec);

    int handle() const;

    virtual QIOEngine *ioEngine() const;

private:
#if defined(Q_DISABLE_COPY)
    QFile(const QFile &);
    QFile &operator=(const QFile &);
#endif
};

#endif // QFILE_H
