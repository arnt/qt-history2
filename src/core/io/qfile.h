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

class QFileEngine;
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

    bool link(const QString &newName);
    static bool link(const QString &oldname, const QString &newName);

    bool copy(const QString &newName);
    static bool copy(const QString &fileName, const QString &newName);

    virtual bool open(int mode);
    bool open(int, FILE *);
    bool open(int, int);
    virtual void close();

#ifdef QT_COMPAT
#if !defined(Q_NO_USING_KEYWORD)
    using QIODevice::at;
#else
    inline QT_COMPAT bool at(Q_LLONG off) { return QIODevice::at(off); }
#endif
#endif

    virtual Q_LLONG size() const;
    virtual Q_LLONG at() const;
    virtual bool seek(Q_LLONG off);

    virtual Q_LLONG read(char *data, Q_LLONG maxlen);
    virtual Q_LLONG write(const char *data, Q_LLONG len);
#if !defined(Q_NO_USING_KEYWORD)
    using QIODevice::write;
#else
    inline QLLONG write(const QByteArray &ba) { return write(data.constData(), data.size()); }
#endif

    virtual void flush();

    virtual int ungetch(int character);

    virtual Q_LLONG readLine(char *data, Q_LLONG maxlen);
    Q_LLONG readLine(QString &string, Q_LLONG maxlen);

    bool resize(QIODevice::Offset sz);
    static bool resize(const QString &filename, QIODevice::Offset sz);

    uint permissions() const;
    static uint permissions(const QString &filename);
    bool setPermissions(uint permissionSpec);
    static bool setPermissions(const QString &filename, uint permissionSpec);

    int handle() const;

    virtual QFileEngine *fileEngine() const;

protected:
    QFile(QFilePrivate &d);

private:
    Q_DISABLE_COPY(QFile)
};

#endif // QFILE_H
