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
    Q_DECLARE_PUBLIC(QDirEngine)
    QDirEngine *q_ptr;
protected:
    inline QDirEnginePrivate() : q_ptr(0) { }
    ~QDirEnginePrivate() { q_ptr = 0; }
};

class QFSDirEngine;
class QFSDirEnginePrivate : public QDirEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSDirEngine)
protected:
    QFSDirEnginePrivate();

private:
    mutable QString path;
};

#endif /* __QDIRENGINE_P_H__ */
