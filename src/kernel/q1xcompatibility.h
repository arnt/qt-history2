/****************************************************************************
** $Id: //depot/qt/main/src/kernel/q1xcompatibility.h#4 $
**
** Various macros etc. to ease porting from Qt 1.x to 2.0.  THIS FILE
** WILL CHANGE OR DISAPPEAR IN THE NEXT VERSION OF QT.
**
** Created : 980824
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef Q1XCOMPATIBILITY_H
#define Q1XCOMPATIBILITY_H


#define Event_None                QEvent::None
#define Event_Timer               QEvent::Timer
#define Event_MouseButtonPress    QEvent::MouseButtonPress
#define Event_MouseButtonRelease  QEvent::MouseButtonRelease
#define Event_MouseButtonDblClick QEvent::MouseButtonDblClick
#define Event_MouseMove           QEvent::MouseMove
#define Event_KeyPress            QEvent::KeyPress
#define Event_KeyRelease          QEvent::KeyRelease
#define Event_FocusIn             QEvent::FocusIn
#define Event_FocusOut            QEvent::FocusOut
#define Event_Enter               QEvent::Enter
#define Event_Leave               QEvent::Leave
#define Event_Paint               QEvent::Paint
#define Event_Move                QEvent::Move
#define Event_Resize              QEvent::Resize
#define Event_Create              QEvent::Create
#define Event_Destroy             QEvent::Destroy
#define Event_Show                QEvent::Show
#define Event_Hide                QEvent::Hide
#define Event_Close               QEvent::Close
#define Event_Quit                QEvent::Quit
#define Event_Accel               QEvent::Accel
#define Event_Clipboard           QEvent::Clipboard
#define Event_SockAct             QEvent::SockAct
#define Event_DragEnter           QEvent::DragEnter
#define Event_DragMove            QEvent::DragMove
#define Event_DragLeave           QEvent::DragLeave
#define Event_Drop                QEvent::Drop
#define Event_DragResponse        QEvent::DragResponse
#define Event_ChildInserted       QEvent::ChildInserted
#define Event_ChildRemoved        QEvent::ChildRemoved
#define Event_LayoutHint          QEvent::LayoutHint
#define Event_ActivateControl     QEvent::ActivateControl
#define Event_DeactivateControl   QEvent::DeactivateControl
#define Event_User                QEvent::User

#define Q_TIMER_EVENT(x)        ((QTimerEvent*)x)
#define Q_MOUSE_EVENT(x)        ((QMouseEvent*)x)
#define Q_KEY_EVENT(x)          ((QKeyEvent*)x)
#define Q_FOCUS_EVENT(x)        ((QFocusEvent*)x)
#define Q_PAINT_EVENT(x)        ((QPaintEvent*)x)
#define Q_MOVE_EVENT(x)         ((QMoveEvent*)x)
#define Q_RESIZE_EVENT(x)       ((QResizeEvent*)x)
#define Q_CLOSE_EVENT(x)        ((QCloseEvent*)x)
#define Q_SHOW_EVENT(x)         ((QShowEvent*)x)
#define Q_HIDE_EVENT(x)         ((QHideEvent*)x)
#define Q_CUSTOM_EVENT(x)       ((QCustomEvent*)x)


#define NoButton	QMouseEvent::NoButton
#define LeftButton	QMouseEvent::LeftButton
#define RightButton	QMouseEvent::RightButton
#define MidButton	QMouseEvent::MidButton
#define MouseButtonMask	QMouseEvent::MouseButtonMask
#define ShiftButton	QMouseEvent::ShiftButton
#define ControlButton	QMouseEvent::ControlButton
#define AltButton	QMouseEvent::AltButton
#define KeyButtonMask	QMouseEvent::KeyButtonMask

// Painter device types (is-A)

#define PDT_UNDEF    	QInternal::UndefinedDevice
#define PDT_WIDGET	QInternal::Widget
#define PDT_PIXMAP	QInternal::Pixmap
#define PDT_PRINTER    	QInternal::Printer
#define PDT_PICTURE	QInternal::Picture
#define PDT_SYSTEM	QInternal::System
#define PDT_MASK	QInternal::DeviceTypeMask

// Painter device flags

#define PDF_EXTDEV	QInternal::ExternalDevice
#define PDF_PAINTACTIVE QInternal::PaintingActive


#endif // Q1XCOMPATIBILITY_H
