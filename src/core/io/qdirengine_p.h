/****************************************************************************
**
** Definition of private QDirEngine classes.
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
#ifndef __QDIRENGINE_P_H__
#define __QDIRENGINE_P_H__

class QDirEngine;
class QDirEnginePrivate
{
protected:
    QDirEngine *q_ptr;
private:
    Q_DECLARE_PUBLIC(QDirEngine)
protected:
    inline QDirEnginePrivate(QDirEngine *qq) : q_ptr(qq) { }
    ~QDirEnginePrivate() { q_ptr = 0; }
};

class QFSDirEngine;
class QFSDirEnginePrivate : public QDirEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSDirEngine)
protected:
    QFSDirEnginePrivate(QFSDirEngine *qq);

    bool sysExists(const QString &path) const;

private:
    mutable QString path;
};

#endif /* __QDIRENGINE_P_H__ */
