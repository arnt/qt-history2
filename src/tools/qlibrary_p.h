/****************************************************************************
**
** Definition of an internal QLibrary class.
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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qlibrary.h"

#ifndef QT_NO_LIBRARY

#ifdef Q_WS_WIN
# include "qwindowdefs_win.h"
#endif


#ifndef QT_H
#endif // QT_H

class QLibraryPrivate
{
public:
    QLibraryPrivate( QLibrary *lib );

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

private:
    QLibrary *library;
};

#endif // QT_NO_LIBRARY
#endif // QLIBRARY_P_H
