/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_p.h#1 $
**
** Definition of some Qt private functions.
**
** Created : 000228
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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

class QWidget;
class QObject;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

extern bool qt_modal_state();
extern void qt_enter_modal( QWidget* );
extern void qt_leave_modal( QWidget* );

extern bool qt_is_gui_used;
extern QObject *qt_clipboard;

#if defined (_OS_WIN32_)
extern Qt::WindowsVersion qt_winver;
#endif

#if defined (_WS_X11_)
extern int qt_ncols_option;
#endif


extern void qt_dispatchEnterLeave( QWidget*, QWidget* );
extern bool  qt_propagateKeyEvent( QWidget* w, QKeyEvent* e );
extern bool  qt_propagateMouseEvent( QWidget* w, QMouseEvent* e );
extern bool  qt_propagateWheelEvent( QWidget* w, QWheelEvent* e );



#endif
