/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAPPLICATION_P_H
#define QAPPLICATION_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp, qcolor_x11.cpp, qfiledialog.cpp
// and many other.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#include "qfont.h"
#include "qcursor.h"
#include "qmutex.h"
#include "qtranslator.h"

#include "qkernelapplication_p.h"
#include "qapplication.h"

class QWidget;
class QObject;
class QClipboard;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

extern Q_EXPORT bool qt_modal_state();
extern Q_EXPORT void qt_enter_modal( QWidget* );
extern Q_EXPORT void qt_leave_modal( QWidget* );

extern bool qt_is_gui_used;
#ifndef QT_NO_CLIPBOARD
extern QClipboard *qt_clipboard;
#endif

#if defined (Q_OS_WIN32) || defined (Q_OS_CYGWIN)
extern Qt::WindowsVersion qt_winver;
enum { QT_TABLET_NPACKETQSIZE = 128 };
# ifdef Q_OS_TEMP
  extern DWORD qt_cever;
# endif
#elif defined (Q_OS_MAC)
extern Qt::MacintoshVersion qt_macver;
#endif

#if defined (Q_WS_X11)
extern int qt_ncols_option;
#endif


extern void qt_dispatchEnterLeave( QWidget*, QWidget* );
extern bool qt_tryModalHelper( QWidget *, QWidget ** = 0 );


class QApplicationPrivate : public QKernelApplicationPrivate
{
    Q_DECL_PUBLIC(QApplication);
public:
    QApplicationPrivate(int &argc, char **argv);
    ~QApplicationPrivate() {}

#ifndef QT_NO_SESSIONMANAGER
    QSessionManager *session_manager;
    QString session_id;
    QString session_key;
    bool is_session_restored;
#endif

    QList<QTranslator*> translators;
#ifndef QT_NO_CURSOR
    QList<QCursor> cursor_list;
#endif
};


#endif
