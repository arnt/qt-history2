/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILEDEFS_P_H
#define QFILEDEFS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfileinfo*.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <qglobal.h>
#include <qatomic.h>
#endif

// Be sure to include qplatformdefs.h first!
class QFileInfoPrivate
{
public:
    QFileInfoPrivate(const QString &file)
	: fn(file), cache(false), could_stat(false), symLink(false) {
	ref = 0;
	slashify(fn);
    }
    QFileInfoPrivate()
	: cache(false), could_stat(false), symLink(false) {
	ref = 0;
    }

    QAtomic ref;

    static bool access( const QString& fn, int t );

    void setFileName(const QString &file) { fn = file; cache = false; }
    QString fileName() const { return fn; }
private:
    QString	fn;

public:
    mutable bool	cache : 1;
    mutable bool        could_stat : 1;
    mutable bool        symLink : 1;
#if defined(Q_WS_WIN)
    mutable QT_STATBUF st;
#else
    mutable struct stat st;
#endif

    void	doStat() const;
#ifdef Q_WS_WIN
    static void slashify( QString & );
    static void makeAbs( QString & );
#else
    inline void slashify( QString & ) {}
    inline void makeAbs( QString & ) {}
#endif
};


#endif
