/****************************************************************************
**
** Definition of some private QDir functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDIR_P_H
#define QDIR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qdir.cpp and qdir_*.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qregexp.h"
#include "qlist.h"
#endif // QT_H

extern bool qt_matchFilterList(const QList<QRegExp> &, const QString &);

extern int qt_cmp_si_sortSpec;

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
extern int __cdecl qt_cmp_si(const void *, const void *);
#else
extern int qt_cmp_si(const void *, const void *);
#endif

#if defined(Q_C_CALLBACKS)
}
#endif


#endif // QDIR_P_H
