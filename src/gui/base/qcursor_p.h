/****************************************************************************
**
** Definition of internal cursor data
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCURSOR_P_H
#define QCURSOR_P_H
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//
#ifndef QT_H
#include <qatomic.h>
# if defined (Q_WS_MAC)
#  include <qt_mac.h>
# elif defined(Q_WS_X11)
#  include <private/qt_x11_p.h>
# elif defined(Q_WS_WIN)
#  include <qt_windows.h>
# endif
#endif

#ifdef Q_WS_MAC
#  if QT_MACOSX_VERSION >= 0x1030
#    define QMAC_USE_BIG_CURSOR_API
#  else
     class QMacCursorWidget;
#  endif
class QMacAnimateCursor;
#endif
class QBitmap;
struct QCursorData {
    QCursorData(int s = 0);
    ~QCursorData();
    QAtomic ref;
    int	      cshape;
    QBitmap  *bm, *bmm;
    short     hx, hy;
#if defined (Q_WS_MAC) || defined(Q_WS_QWS)
    int id;
#endif
#if defined (Q_WS_WIN)
    HCURSOR   hcurs;
#elif defined (Q_WS_X11)
    XColor    fg, bg;
    Cursor    hcurs;
    Pixmap    pm, pmm;
#elif defined (Q_WS_MAC)
    enum { TYPE_None, TYPE_CursorImage, TYPE_CursPtr, TYPE_ThemeCursor, TYPE_FakeCursor, TYPE_BigCursor } type;
    union {
	struct {
	    uint my_cursor:1;
	    CursPtr   hcurs;
	} cp;
#ifndef QMAC_NO_FAKECURSOR
	struct {
	    QMacCursorWidget *widget;
	    CursPtr empty_curs;
	} fc;
#endif
#ifdef QMAC_USE_BIG_CURSOR_API
	char *big_cursor_name;
#endif
	CursorImageRec *ci;
	struct {
	    QMacAnimateCursor *anim;
	    ThemeCursor curs;
	} tc;
    } curs;
#endif
};

#endif
