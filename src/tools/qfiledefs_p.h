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

// Be sure to include qplatformdefs.h first!
struct QFileInfoCache
{
#if defined(Q_WS_WIN)
    QT_STATBUF st;
#else
    struct stat st;
#endif
};


#endif
