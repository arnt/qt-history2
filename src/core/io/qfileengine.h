/****************************************************************************
**
** Definition of QFileEngine and QFSFileEngine classes.
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
#ifndef __QFILEENGINE_H__
#define __QFILEENGINE_H__

#include "qfile.h"

class QFileEnginePrivate;
class QFileEngine
{
protected:
    QFileEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileEngine)
public:
    QFileEngine(QFileEnginePrivate &);
    virtual ~QFileEngine();

    virtual bool isOpen() const = 0;
    virtual bool open(int mode, const QString &file) = 0;
    virtual bool close() = 0;
    virtual void flush() = 0;

    virtual Q_LONG readBlock(char *data, Q_ULONG len) = 0;
    virtual Q_LONG writeBlock(const char *data, Q_ULONG len) = 0;
    
    virtual bool remove(const QString &) = 0;

    virtual QFile::Offset size() const = 0;
    virtual QFile::Offset at() const = 0;
    virtual bool atEnd() const = 0;
    virtual bool seek(QFile::Offset pos) = 0;
    virtual bool isSequential() const = 0;

    virtual int handle() const = 0;

    //maybe
    virtual uchar *map(Q_ULONG len) = 0; //can we implement a mmap?
};

class QFileEngineHandler
{
protected:
    QFileEngineHandler();
    virtual ~QFileEngineHandler();

    virtual QFileEngine *createFileEngine(const QString &path) = 0;

private:
    friend QFileEngine *qt_createFileEngine(const QString &);
};

class QFSFileEnginePrivate;
class QFSFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QFSFileEngine();

    virtual bool isOpen() const;
    virtual bool open(int mode, const QString &file);
    bool open(int fd); //FS only!!
    virtual bool close();
    virtual void flush();

    virtual Q_LONG readBlock(char *data, Q_ULONG len);
    virtual Q_LONG writeBlock(const char *data, Q_ULONG len);
    
    virtual bool remove(const QString &);

    virtual QFile::Offset size() const;
    virtual QFile::Offset at() const;
    virtual bool atEnd() const;
    virtual bool seek(QFile::Offset pos);
    virtual bool isSequential() const;
    
    virtual int handle() const;

    //maybe
    virtual uchar *map(Q_ULONG len);
};
#endif /* __QFILEENGINE_H__ */
