/****************************************************************************
**
** Definition of QFile class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef __QFILE_H__
#define __QFILE_H__


#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QFilePrivate;

class Q_CORE_EXPORT QFile : public QIODevice                        // file I/O device class
{
    Q_DECLARE_PRIVATE(QFile)

public:
    QFile();
    QFile(const QString &name);
    ~QFile();

    QString name() const;
    void setName(const QString &name);

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

    bool open(int);
    bool open(int, FILE *);
    bool open(int, int);
    void close();
    void flush();

    Offset size() const;
    Offset at() const;
    bool at(Offset);
    bool atEnd() const;

    virtual Q_LONG readBlock(char *data, Q_LONG len);
    virtual Q_LONG writeBlock(const char *data, Q_LONG len);
#ifdef Q_NO_USING_KEYWORD
    inline Q_LONG writeBlock(const QByteArray &ba) { return QIODevice::writeBlock(ba); }
#else
    using QIODevice::writeBlock;
#endif
    virtual Q_LONG readLine(char *data, Q_LONG maxlen);
    Q_LONG readLine(QString &string, Q_LONG maxlen);

    int getch();
    int putch(int);
    int ungetch(int);

    int handle() const;

private:
#if defined(Q_DISABLE_COPY)
    QFile(const QFile &);
    QFile &operator=(const QFile &);
#endif
};

#endif /* __QFILE_H__ */
