/****************************************************************************
**
** Definition of private QFileEngine classes.
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
#ifndef __QFILEENGINE_P_H__
#define __QFILEENGINE_P_H__

class QFileEngine;
class QFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QFileEngine)
    QFileEngine *q_ptr;
protected:
    inline QFileEnginePrivate() : q_ptr(0) { }
    ~QFileEnginePrivate() { q_ptr = 0; }
};

class QFSFileEngine;
class QFSFileEnginePrivate : public QFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)
protected:
    QFSFileEnginePrivate();

    void init();
    int sysOpen(const QString &, int flags);
private:
    QString file;

    int fd;
    mutable uint sequential : 1;
    mutable uint hasCachedChar : 1;
    mutable uchar cachedCharRead;
};

#endif /* __QFILEENGINE_P_H__ */
