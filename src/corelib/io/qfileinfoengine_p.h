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

#ifndef QFILEINFOENGINE_P_H
#define QFILEINFOENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qplatformdefs.h>

class QFileInfoEngine;
class QFileInfoEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QFileInfoEngine)
    QFileInfoEngine *q_ptr;
protected:
    inline QFileInfoEnginePrivate() : q_ptr(0) { }
    ~QFileInfoEnginePrivate() { q_ptr = 0; }
};


class QFSFileInfoEngine;
class QFSFileInfoEnginePrivate : public QFileInfoEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileInfoEngine)
protected:
    QFSFileInfoEnginePrivate();

    void init();
    void slashify();
private:
    QString file;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
    mutable QT_STATBUF st;
    bool doStat() const;

#if defined(Q_OS_WIN32)
    uint getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFILEINFOENGINE_P_H
