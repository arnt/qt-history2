/****************************************************************************
** $Id$
**
** Implementation of QTestControl class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt TestFramework of the Qt GUI Toolkit.
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

#include <qapplication.h>
#include <qdatetime.h>
#include <qcursor.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <qobjectlist.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qregexp.h>
#include "private/qtestcontrol_p.h"
#include "private/qtestlistbox_p.h"
#include "private/qtestmenubar_p.h"
#include "private/qtestpopupmenu_p.h"
#include <qtimer.h>
#include <qmessagebox.h>
#include <qsocketdevice.h>

/*!
  \internal
  \class QTestControl qtestcontrol.h
  \brief The QTestControl class is an extension to QRemoteControl and introduces
  functionality for remote control (testing) of a Qt application.
  
    Detailed description

   <strong>Groups of functions:</strong>
  <ul>

  <li> Construction:
	QTestControl(),
	~QTestControl(),
	lock(),
	release().

  <li> Misc:
        open(),
	close(),

  <li> Message handlers:
	handleNotification(), 
	postObject(),
	sendObject().
  </ul>

  \link testing.html Introduction to testing \endlink
  \link qttestframework.html Qt Test-Framework overview \endlink
  \link qvalidator.html Qt Validator user guide \endlink
*/

/*!
  \internal
    Creates and initialises an instance of QTestControl.
    Never create an instance directly, but instead use lock() to get a reference to
    the singleton instance.
*/

QTestControl::QTestControl()
{
    m_remoteClient = 0;

    isSimulatedEvent = FALSE;
    is_recording = FALSE;
    is_replaying = FALSE;
    windowDumpKey = Qt::Key_F12;
    widgetDumpKey = Qt::Key_F11;
    nameDelimiterChar = '#';
    anonymous_magic_ = "__anonymous__";
    playFileCount = 1;
    recFileCount = 1;
    replayVersion = 12;
    widget_scale_dict_ = 0;
    replay_file_qt_version_ = 0;
    curMessage = new QRemoteMessage;
    m_replyList.setAutoDelete(FALSE);
    m_cmdList.setAutoDelete(FALSE);
    m_socket = new QSocket;

    nameDelimiterString = "";
    nameDelimiterString += nameDelimiterChar;
    qDebug("QTestControl created.....");
}

/*!
  \internal
    Destructs the instance of QTestControl. Never call the destructor directly.
    Instead use release() to release the reference to the singleton instance.
*/

QTestControl::~QTestControl()
{
    close();

    delete curMessage;
    delete m_socket;

    qDebug("QTestControl destroyed.....");
}

/*!
  \internal
    Connect a remote \a client to the remote control.
    A remote client is a piece of code inside your application that is able to receive
    and handle commands send by the remote control.
*/

void QTestControl::setRemoteClient( QRemoteClient *client ) 
{ 
    m_remoteClient = client; 
}

/*!
  \internal
    Opens a socket connection to the specified \a hostname and \a port.
    After a connection is successfully opened the instance will listen and respond
    to incoming commands received from the remote controller, it will send reply
    messages back to the remote controller and if recording is on it will send all
    user generated events to the remote controller.
*/

void QTestControl::open(const QString& hostname, int port)
{
    close();

    recFileCount = 1;
    m_socket->connectToHost(hostname,port);

    connect(m_socket,SIGNAL(hostFound()),this,SLOT(hostFound()));
    connect(m_socket,SIGNAL(connected()),this,SLOT(hostConnected()));
    connect(m_socket,SIGNAL(error(int)),this,SLOT(error(int)));
    connect(m_socket,SIGNAL(connectionClosed()),this,SLOT(hostClosed()));
    connect(m_socket,SIGNAL(delayedCloseFinished()),this,SLOT(hostClosed()));
    connect(m_socket,SIGNAL(readyRead()),this,SLOT(onData()));
}

/*!
  \internal
    Closes an open socket connection to the remote controller.
*/

void QTestControl::close()
{
    stopRecording();
    stopReplay();

    scalingInfo.clear();
    widget_scale_dict_.clear();
/*
    delete widget_replay_name_list_;
    widget_replay_name_list_ = 0;

    delete widget_scale_dict_;
    widget_scale_dict_ = 0;

    scaling_infos_.setAutoDelete (true);
    scaling_infos_.clear();
*/
    m_socket->close();
}

/*!
  \internal
    Returns the connection status to the TestServer.
    A return value of TRUE means the connection is established.
*/

bool QTestControl::isOpen()
{
    return m_socket->isOpen();
}

/*!
  \internal
    Reads the remote control connection and responds to received commands.
*/

void QTestControl::onData()
{
    while (m_socket->bytesAvailable() > 0) {

	if (curMessage->receive(m_socket)) {

	// append message to stack if its a reply
	    if (curMessage->isReply()) {

		m_replyList.append(curMessage);
		// create a new curMessage
		curMessage = new QRemoteMessage;
	    } else {

		m_cmdList.append(curMessage);
		// create a new curMessage
		curMessage = new QRemoteMessage;

		// pass the remote message on trough the activation of an event
		// to break the direct link between the current event handler and 
		// the processing of the message.
		QTimer::singleShot(0,this,SLOT(onCommand()));
	    }
			
	} else {

	    // build in safetynet to make sure we receive everything...
	    if (m_socket->size() > 0)
		QTimer::singleShot(50,this,SLOT(onData()));
	    return;

	}
    }
}

/*!
   \internal
    Signals the instance that a remote command is available.
*/

void QTestControl::onCommand()
{
    QRemoteMessage* msg = m_cmdList.first();
    if (msg) {

	handleCommand(msg);
	m_cmdList.removeFirst();
//	delete msg; dont delete, we use autodelete for m_cmdList
    }
}

/*!
   \internal
    Handles received remote commands.
*/

void QTestControl::handleCommand(QRemoteMessage *msg)
{
    if (msg->event() == "SimulateEvent") {
qDebug("Receive command: " + msg->event());
	if (simulateEvent(msg)) {
	    QRemoteMessage S("ACK","");
	    S.send(m_socket);
	} else {
	    // No need to give more info here, this is already done in simulateEvent()
	    QRemoteMessage S("NAK",""); 
	    S.send(m_socket);
	}
	return;
    } else if (msg->event() == "MouseInfo") {

	QPoint gp = QCursor::pos ();
	QString curWidgetName;
	QString curClassName;
	QString localMousePos;
	QString absMousePos;

	const QWidget* widget_under_mouse = QApplication::widgetAt(gp, TRUE);
	if (widget_under_mouse) {
	    curWidgetName = widget_under_mouse->name();
	    curClassName = widget_under_mouse->className();
	    QPoint local_xy (widget_under_mouse->mapFromGlobal (gp));
	    localMousePos.sprintf("%d,%d",local_xy.x(),local_xy.y());
	} else {
	    curWidgetName = "";
	    curClassName = "";
	    localMousePos = "*, *";
	}
	absMousePos.sprintf("%d,%d",gp.x(),gp.y());

	QRemoteMessage S("MouseInfo",curClassName + " " + localMousePos + " " + absMousePos + " " + curWidgetName);
	S.send(m_socket);
	return;
    } else if (msg->event() == "WidgetDef") {

	QString S = msg->message();
//qDebug("WidgetDef: " + S);
	addWidgetDef(S);
	return;
    } else if (msg->event() == "WidgetDefStart") {

	widgetDefs.clear();
	return;
    } else if (msg->event() == "WidgetDefEnd") {

	// Signal the other side that we start sending widgetdefs
	QRemoteMessage s("WidgetDefStart","");
	s.send(m_socket);

        QObjectList* search_list = (QObjectList*)(QApplication::topLevelWidgets());  // delete
	updateWidgetDefs(search_list);
        delete search_list;

	// Signal the other side that we are ready
	QRemoteMessage s2("WidgetDefEnd","");
	s2.send(m_socket);
	return;
    } else if (msg->event() == "AppId") {
	
	QUuid id = qApp->applicationId();

	// Signal the other side the id of the application
	QRemoteMessage s2("AppId",id.toString());
	s2.send(m_socket);
	return;

    } else if (msg->event() == "Run") {

	if (m_remoteClient != 0) {
	    QByteArray *b;
	    assert(msg->getData(b));
	    
	    if (!m_remoteClient->execCommand(b)) {

		QRemoteMessage S("NAK","Remote client didn't accept command.");
		S.send(m_socket);
		return;
	    }
	} else {
	    QRemoteMessage S("NAK","No remote client to send command to.");
	    S.send(m_socket);
	    return;
	}

	QRemoteMessage S("ACK","");
	S.send(m_socket);
    } else if (msg->event() == "StartRecording") {

	startRecording();
	return;
    } else if (msg->event() == "StopRecording") {

	stopRecording();
	return;
    } else if (msg->event() == "StartReplay") {

	startReplay();
	QRemoteMessage S("ACK","");
	S.send(m_socket);
	return;
    } else if (msg->event() == "StopReplay") {

	stopReplay();
	return;
    } else if (msg->event() == "Terminate") {

	QRemoteMessage S("ACK","");
	S.send(m_socket);

	if (qApp)
	    qApp->quit();
	else {
	    m_socket->close();
	    exit(0);
	}
	return;
/*
    } else if (msg->event() == "TopLeftScaling") {

	assert(FALSE);

	QByteArray *b;
	assert(msg->getByteArray(b));
        QTestScalingInfo* si = new QTestScalingInfo(b, QTestScalingInfo::TopLeftScaling);
        if (si)
            scalingInfo.append(si);
*/
    } else {

	QMessageBox box( 0, "qt_testserver_msgbox" );
	box.setText(" Received an unknown event \n"+msg->event() + "\n" + msg->message());
	box.exec();

	QRemoteMessage S("NAK","");
	S.send(m_socket);
	return;
    }

/*
	} else if (event_name == "_Sync") {

	    unsigned long sync_value;
	    double        clock_seconds_diff = 0;
	    line_stream >> sync_value;
	    line_stream >> clock_seconds_diff;
	    replay_sync_wait_loops_ = sync_value * sync_wait_fudge_ / 100;
	    replay_sync_wait_clock_ = ::clock() + (clock_t)(clock_seconds_diff * CLOCKS_PER_SEC * clock_wait_fudge_);
	} else if (event_name == "_SyncFudge") {

	    line_stream >> sync_wait_fudge_;
	} else if (event_name == "_ClockFudge") {

	    line_stream >> clock_wait_fudge_;
	} else if (event_name == "_If") {

	    QString condition;
	    condition = line_stream.readLine().mid(1);
	    handleIf(condition);
	} else if (event_name == "_ElseIf") {

	    QString condition;
	    condition = line_stream.readLine().mid(1);
	    handleElseIf(condition);
	} else if (event_name == "_Else") {

	    handleElse ();
	} else if (event_name == "_EndIf") {

	    handleEndIf ();
*/
}

/*!
   \internal
    Injects an event specified by \a msg into the event loop.
*/

bool QTestControl::simulateEvent( QRemoteMessage *msg )
{
    QString tmp = msg->message();
    QTextStream line_stream(tmp, IO_ReadOnly);
    
    int event_id;
    line_stream >> event_id;

    int  widget_id;
    line_stream >> widget_id;
    
    QCString widget_name;
    if (!widgetDefs.findWidgetName( widget_id, widget_name )) {

	QString s;
	s.sprintf( "Unknown widget index (%d)", widget_id );

	QRemoteMessage S("ERR",s);
	S.send(m_socket);
	return FALSE;
    }

    QString missing_name;  // set if widget is not found
    QObject* event_widget = findAppWidgetByName( widget_name, missing_name, FALSE );

    if (!event_widget)
	event_widget = findGlobalWidget( widget_name );

    if (event_widget) {

        QEvent* event = 0;
        switch (event_id)
        {
            case QEvent::MouseMove:
                event = readMouseEvent((QEvent::Type)event_id, line_stream, event_widget, widget_id);
                break;

            case QEvent::MouseButtonPress:
	    {
                event = readMouseEvent((QEvent::Type)event_id, line_stream, event_widget, widget_id);
		if (qApp && event_widget->isWidgetType ())
                {
                    QFocusEvent::Reason old_reason = QFocusEvent::reason();
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    ((QWidget*)event_widget)->setFocus();
		    QFocusEvent::setReason(old_reason);
		    ((QWidget*)event_widget)->setActiveWindow();
                }
                break;
	    }
            case QEvent::MouseButtonRelease:
                event = readMouseEvent((QEvent::Type)event_id, line_stream, event_widget, widget_id);
                break;

            case QEvent::MouseButtonDblClick:
                event = readMouseEvent((QEvent::Type)event_id, line_stream, event_widget, widget_id);
                break;

            case QEvent::Move:
                event = readMoveEvent(line_stream, event_widget);
                break;

            case QEvent::Resize:
                event = readResizeEvent(line_stream, event_widget);
                break;

            case QEvent::KeyPress:
            case QEvent::KeyRelease:
            case QEvent::Accel:
                event = readKeyEvent((QEvent::Type)event_id, line_stream);
                break;

            case QEvent::Close:
                event = new QCloseEvent;
                break;

            case QEvent::Show:
                // do nothing, the widget is already displayed if we get here
                assert (event_widget->isWidgetType ());
                assert (((QWidget*)event_widget)->isVisible ());
                break;

            default:
		QString err_msg;
		err_msg.sprintf("Unexpected event: %d", event_id);
                QRemoteMessage S("ERR",err_msg);
		S.send(m_socket);
		break;
        }

        if (event) {

            notify(event_widget, event);
	    delete event;
	    return TRUE;
        } else {

            delete event;
	    return FALSE;
	}
    }

    if ( widget_name == "#" ) {     // ignore error for this widget

	QRemoteMessage S("WRN","Ignoring widget named '#'");
	S.send(m_socket);
	return TRUE; // I think we can go on here ??
    } 
	
    QString name_trail = widget_name.right(12);
    if ((name_trail == "QScrollBarv#") || (name_trail == "QScrollBarh#")) {

	QRemoteMessage S("WRN","Scrollbars may not appear if font too small, event ignored");
	S.send(m_socket);
        return TRUE; // We can go on here
    }

    name_trail = widget_name.right(9);
    if ((name_trail == "#qt_hbar#") || (name_trail == "#qt_vbar#")) {

	QRemoteMessage S("WRN","Scrollbars may not appear if font too small, event ignored");
	S.send(m_socket);
        return TRUE; // We can go on here
    }

    if (event_id == QEvent::MouseMove) {

        // X11 window managers like to create invalid mouse moves, so just ignore
        event_widget = findAppWidgetByName( widget_name, missing_name, TRUE);
        if (event_widget) {

            QMouseEvent* event = readMouseEvent((QEvent::Type)event_id, line_stream, event_widget, widget_id);
            if (event && (event->state() & (Qt::LeftButton | Qt::RightButton | Qt::MidButton))) {

		QRemoteMessage S("WRN","Invalid mouse move ignored");
		S.send(m_socket);
		return TRUE;
            }
        }
    } else if (event_id == QEvent::Move) {

        event_widget = findAppWidgetByName(widget_name, missing_name, TRUE);
        if (event_widget) {

            QEvent* event = readMoveEvent (line_stream, event_widget);
            if (event && qApp) {

                notify(event_widget, event);
		delete event;
		return TRUE;
            }
        }
    } else if (event_id == QEvent::Resize) {

        event_widget = findAppWidgetByName(widget_name, missing_name, TRUE);
        if (event_widget) {

            QEvent* event = readResizeEvent (line_stream, event_widget);
            if (event && qApp) {

                notify(event_widget, event);
		delete event;
		return TRUE;
            }
		}
    } else if (event_id == QEvent::MouseButtonDblClick) {

        event_widget = findAppWidgetByName (widget_name, missing_name, TRUE);
        if (event_widget) {

            if (( ! event_widget->parent () && event_widget->inherits ("QListBox"))
                || isWidgetAComboBoxPanel (event_widget)) {

                // this event cannot be done by the user, but is created by Qt itself
                QRemoteMessage S("ERR","Invalid MouseButtonDblClick event for option menu.");
		S.send(m_socket);
		return TRUE; // I think we can continue here ??!!
            } else {

                QMouseEvent* event = readMouseEvent ((QEvent::Type)event_id, line_stream, event_widget, widget_id);
                if (event && qApp) {

                    notify(event_widget, event);
		    delete event;
		    return TRUE;
                }
            }
        }
    } else if (event_id == QEvent::MouseButtonRelease) {

        // X11 window managers like to create invalid mouse events, allow to send this one to already hidden widget
        event_widget = findAppWidgetByName (widget_name, missing_name, TRUE);
        if (event_widget) {

            QMouseEvent* event = readMouseEvent ((QEvent::Type)event_id, line_stream, event_widget, widget_id);
            if (event && qApp) {

                notify(event_widget, event);
		delete event;
		return TRUE;
            }
        }
    } else if (event_id == QEvent::KeyRelease) {

        // window could have been undisplayed by KeyPress event, so allow to send to invisible widget
        event_widget = findAppWidgetByName (widget_name, missing_name, TRUE);
        if (event_widget) {

            QKeyEvent* event =  readKeyEvent ((QEvent::Type)event_id, line_stream);
            if (event && qApp) {

                notify(event_widget, event);
		delete event;
		return TRUE;
            }
        }
    }

    if (!event_widget)
        event_widget = findAppWidgetByName(widget_name, missing_name, TRUE);

    if (event_widget) {

	QCString wName;
	widgetDefs.findWidgetName( widget_id, wName );

	QString msg = "Widget '" + missing_name + "' in '";
	msg+= wName;
	msg+= "' not visible.";

        QRemoteMessage S("ERR",msg);
	S.send(m_socket);
    } else {

	QCString wName;
	widgetDefs.findWidgetName( widget_id, wName );

	QString msg = "Unknown widget '" + missing_name + "' in '";
	msg+= wName;
	msg+= "'";

        QRemoteMessage S("ERR",msg);
	S.send(m_socket);
    }

    return FALSE;
}

/*!
   \internal
    Signals the instance that a remote control host is found.
*/

void QTestControl::hostFound()
{
}

/*!
   \internal
    Signals the instance that a remote control host has closed the connection.
*/

void QTestControl::hostClosed()
{
    close();
}

/*!
   \internal
    Signals the instance that a connection is established with a remote control host.
*/

void QTestControl::hostConnected()
{
}

/*!
   \internal
    Signals the instance that user generated events must be send to the remote controller
*/

void QTestControl::startRecording()
{
    is_recording = TRUE;
}

/*!
   \internal
    Browses recursive through the \a list and assigns all available widgets with an 
    unique ID that will be shared with the test server.
*/

void QTestControl::updateWidgetDefs( const QObjectList* list )
{
    if (list != 0) {

	uint count = list->count();
        QObjectListIt it (*list);
        while (count > 0 && it.current()) {
	    count--;

	    if (it.current()->isWidgetType()) {
//		&& (!((QWidget*)it.current())->isTopLevel())) {

	        QCString widget_name;
		if (verifyUniqueName( it.current(), widget_name )) {

		    // only append widgets to the defs if the name is unique
		    int widget_index;
		    if (!widgetDefs.findWidgetIndex( widget_name, widget_index )) {

			QString s = widgetDefs.appendWidget( widget_name );
			QRemoteMessage S( "WidgetDef", s );
			S.send( m_socket );
		    }
		}
	    }
	    
	    // append all children too
	    updateWidgetDefs( it.current()->children() );

            ++it;
        }
    }
}

/*!
   \internal
    Signals the instance that it must stop sending user generated events to the remote control.
*/

void QTestControl::stopRecording()
{
    if (is_recording) {
	is_recording = FALSE;
    }
}

/*!
   \internal
    Signals the instance that it can expect events from the remote controller.
*/

void QTestControl::startReplay()
{
    if (is_replaying)
	return;

    playFileCount = 1;
    is_replaying = TRUE;
}

/*!
   \internal
    Signals the instance that the remote control has stopped sending events to the instance.
*/

void QTestControl::stopReplay()
{
    if (is_replaying) {
	is_replaying = FALSE;
    }
}

/*!
   \internal
    Signals the instance that the remote connection has experienced an \a error.
*/

void QTestControl::error( int error )
{
    if (error == QSocket::ErrConnectionRefused)
	qDebug("Connection refused");
    else
	qDebug("error %d",error);
}

/*!
   \internal
    Replaces QApplication::notify.
*/

void QTestControl::notify(QObject* event_widget, QEvent* event)
{
    if (!qApp)
	return;

    isSimulatedEvent = TRUE;
    qApp->notify(event_widget, event);
    isSimulatedEvent = FALSE;
}

/*!
   \internal
    Sends event \a e from widget \a receiver to the remote controller if recording is on.
    Returns FALSE if the caller (e.g. QApplication::notify) should still handle
    the event.
*/

bool QTestControl::handleNotification( QObject *receiver, QEvent *e )
{
    // if we generate an event ourselves (e.g. we simulate it), we obviously don't want to record it again
    // So let QApp handle it without further delay
    if (isSimulatedEvent)
	return FALSE; 

    if (is_replaying)
    {
	if ( 
		(e->type() == QEvent::MouseMove)
          //|| ((e->type () == QEvent::Move)   && ! isToplevelEventFromUser (receiver))
          //|| ((e->type () == QEvent::Resize) && ! isToplevelEventFromUser (receiver))
           ) {

	    // returning TRUE means QTestControl has handled the notification. 
	    // i.e. QApplication shouldn't handle it anymore
	    return TRUE; 
        }
    }

    if (is_recording) {
	recordEvent(receiver, e);
    }

    return FALSE; // Let QApplication::notify() handle the notification too
}

/*!
   \internal
    Converts event \a e referring to object \a receiver into a binary stream and sends
    it to the testserver.
*/

void QTestControl::recordEvent( QObject* receiver, QEvent* e )
{
    assert(e);

    if (!e->spontaneous()) {
	return;
    }

    if (mustWriteEvent(e, receiver)) {

	Q_ASSERT(receiver->isWidgetType());

        QString event_line;
        switch (e->type ())
        {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                getEventLine((QMouseEvent*)e, receiver, event_line);
                break;

            case QEvent::MouseButtonDblClick:
                // this event cannot be done by the user, but is created by Qt itself
                assert(!isWidgetAComboBoxPanel(receiver));
                getEventLine((QMouseEvent*)e, receiver, event_line);
                break;

            case QEvent::Move:
//                assert(!isToplevelEventFromUser (receiver));
                if (!getEventLine((QMoveEvent*)e, ((QWidget*)receiver)->pos(), event_line))
		    return; // old and new pos are equal
                break;

            case QEvent::Resize:
//                assert(!isToplevelEventFromUser (receiver));
                if (!getEventLine((QResizeEvent*)e, event_line))
		    return; // old and new size are equal
                break;

            case QEvent::KeyPress:
                // must we dump a screen?
            {
                QKeyEvent* key_event = (QKeyEvent*)e;
                if ((key_event->state () & (Qt::ShiftButton | Qt::ControlButton)) == (Qt::ShiftButton | Qt::ControlButton))
                {
                    QWidget* dump_widget = 0;
                    if (key_event->key () == windowDumpKey)
                    {
                        dump_widget = QApplication::widgetAt (QCursor::pos (), FALSE);
                        dump_widget = dump_widget->topLevelWidget ();  // work around bug in Qt 1.x & 2.0
                    }
                    else if (key_event->key () == widgetDumpKey)
                        dump_widget = QApplication::widgetAt (QCursor::pos (), TRUE);

                    if (dump_widget)
                    {
                        QString dump_filename = "_pxmdump_";
                        int ext_pos = dump_filename.findRev ('.');
                        if (ext_pos > 0)
                            dump_filename.truncate (ext_pos);
                        dump_filename += "_rec_";
                        dump_filename += QString ().setNum (recFileCount);
                        dumpPixmapToFile (*dump_widget, dump_filename);
                        ++recFileCount;              
                    }
                }
            }
            case QEvent::KeyRelease:
            case QEvent::Accel:
                getEventLine ((QKeyEvent*)e, event_line);
                break;

            case QEvent::Show: // edba: do we need this????
                break;

	    default:
		return; // don't write anything in all other cases
		break;
        }

        QCString widget_name;
	if (!verifyUniqueName(receiver, widget_name ))
	    return;

        int widget_index = -1;
	if (!widgetDefs.findWidgetIndex( widget_name, widget_index )) {

	    if (receiverIsAccessible(receiver)) {

		QString s = widgetDefs.appendWidget( widget_name );
		QRemoteMessage S("WidgetDef",s);
		S.send(m_socket);

		// append all children too
		updateWidgetDefs( receiver->children() );

	    } else {

		QRemoteMessage S("ERR","Widget '" + widget_name + "' is not accessible.");
		S.send(m_socket);
	    }

        }

	if (widget_index == -1) {

	    QRemoteMessage S("ERR","Widget '" + widget_name + "' is not accessible anymore.");
	    S.send(m_socket);
	} else {

	    QString s;
	    s.sprintf("%d %d %s",e->type(),widget_index, event_line.latin1());

	    QRemoteMessage S("Event",s);
	    S.send(m_socket);
	}
    }
/*
    if ((e->type() == QEvent::Enter)
        && (is_recording || is_replaying)
        && receiver->inherits ("QComboBox"))
    {
        //
        // make sure the ComboBox popup has a name
        //
        QListBox* listbox = ((QComboBox*)receiver)->listBox ();
        if (listbox && !hasName(listbox))
        {
            listbox->setName(getFullName(receiver) + "__popup__");
	    txtStream << "_WRN: Giving an unknown combobox a name" << endl << flush;
        }
    }
*/
}

bool QTestControl::appendWidget(const QCString &widget_name, int widget_index)
{
    bool indexError;
    if (!widgetDefs.appendWidget( widget_name, widget_index, indexError)) {
	
	QString msg;
	if (indexError)
	    msg = "Widget index is already defined!";
	else
	    msg = "Widget name is already defined!";

	QRemoteMessage S("WRN",msg);
	S.send(m_socket);
	return FALSE;
    }

    return TRUE;
}

bool QTestControl::mustWriteEvent(QEvent* e, QObject* receiver)
{
    if (receiver == 0)
	return FALSE;

    QString name = receiver->name();
    // first check if the event is generated by a known widget-type we don't want to record
    if (name == "toolTipTip") {
	return FALSE;
    }
    if (name == "qt internal roll effect widget") {
	return FALSE;
    }
//    if (name == "qt internal alpha effect widget") {
//	return FALSE;
//    }

    // these events are handled by main plane
    if (receiver->inherits("QGLOverlayWidget")) {
	return FALSE;
    }

    switch (e->type ())
    {
	case QEvent::MouseMove:
	    return doesWidgetExistRightNow(receiver);  // X11 may report move events for deleted widgets

	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::Move:
	case QEvent::Resize:
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::Close:
	case QEvent::Quit:
	case QEvent::Accel:
	    return TRUE;

	case QEvent::Show:
	    return receiver->inherits("QPopupMenu");
	default:
	    break;
    }
    return FALSE;
}

void QTestControl::getEventLine(QKeyEvent* e, QString& event_line)
{
    assert (e);

    QString event_data;
    event_data.sprintf (" %d %d %d %d %d %d"
                        , e->key ()
                        , e->ascii ()
                        , e->state ()
                        , e->isAutoRepeat ()
                        , e->count ()
                        , e->text ().length ()
                       );
    const int text_length = e->text ().length ();
    for (int c = 0; c < text_length; ++c)
    {
        event_data += ' ';
        event_data += QString::number (e->text ().constref (c).unicode ());
    }
    event_line += event_data;
}

void QTestControl::getEventLine(QMouseEvent* e, const QObject* event_widget, QString& event_line)
{
    assert (e);
    assert (event_widget);

    int menu_item = -1;
    QCString full_listviewitem_name;
    QCString menu_item_text = "";
    if (event_widget->inherits ("QMenuBar"))
    {
        ((QTestMenuBar*)event_widget)->mousePos2Index(e->pos(), menu_item);
//			txtStream << "_ERR: mouse position does not correspond with menubar item" << endl << flush;
    }
    else if (event_widget->inherits ("QPopupMenu"))
    {
        QTestPopupMenu* pm = (QTestPopupMenu*)event_widget;
        pm->mousePos2Index (e->pos(), menu_item);
//			txtStream << "_ERR: mouse position does not correspond with popupmenu item" << endl << flush;
        menu_item_text = pm->text (pm->idAt (menu_item)).latin1 ();
    }
    else if (event_widget->inherits ("QListBox"))
    {
        QListBoxItem* item = ((QListBox*)event_widget)->itemAt (e->pos ());
        if (item)
        {
            menu_item = ((QListBox*)event_widget)->index (item);
            menu_item_text = ((QListBox*)event_widget)->text (menu_item).latin1 ();
        }
    }
    else if (event_widget->parent ()
            && event_widget->parent ()->inherits ("QListView")
            && ! event_widget->inherits ("QScrollBar"))
    {
        QListViewItem* item = ((QListView*)(event_widget->parent ()))->itemAt (e->pos ());
        if (item)
        {
            for (QListViewItem* i = item; i; i = i->parent ())
            {
                full_listviewitem_name.prepend (nameDelimiterString);
                full_listviewitem_name.prepend (i->text (0).latin1 ());
            }
            menu_item = -2;
        }
    }
    else if (isWidgetAListBoxArea (event_widget) || isWidgetAComboBoxPanel (event_widget))
    {
        QListBoxItem* item = ((QListBox*)(event_widget->parent ()))->itemAt (e->pos ());
        if (item)
        {
            menu_item = ((QListBox*)(event_widget->parent ()))->index (item);
            menu_item_text = item->text ().latin1 ();
        }
    }
    QString event_data;
    event_data.sprintf(" %4d %4d %d %d %d %s"
                       , e->x ()
                       , e->y ()
                       , e->button ()
                       , e->state ()
                       , menu_item
                       , (const char*)menu_item_text);
    if (menu_item == -2)
    {
        event_data += ' ';
        event_data += full_listviewitem_name;
    }
    event_line += event_data;
}

bool QTestControl::getEventLine(QMoveEvent* e, const QPoint& widget_pos, QString& event_line)
{
    assert (e);

    if (e->pos() != e->oldPos ())
    {
        QString event_data;
        event_data.sprintf (" %4d %4d %4d %4d"
                           , widget_pos.x ()
                           , widget_pos.y ()
                           , e->oldPos ().x ()
                           , e->oldPos ().y ()
                           );
        event_line += event_data;
        return TRUE;
    }
    return FALSE;
}

bool QTestControl::getEventLine(QResizeEvent* e, QString& event_line)
{
    assert (e);

    if (e->size () != e->oldSize ())
    {
        QString event_data;
        event_data.sprintf (" %4d %4d %4d %4d"
                           , e->size ().width ()
                           , e->size ().height ()
                           , e->oldSize ().width ()
                           , e->oldSize ().height ()
                           );
        event_line += event_data;
        return TRUE;
    }
    return FALSE;
}

bool QTestControl::isWidgetAComboBoxPanel(const QObject* event_widget)
{
    assert (event_widget);
    if ( ! event_widget)
        return FALSE;

    return ((::strcmp (event_widget->name (), "qt_viewport") == 0)
            && event_widget->isA ("QWidget")
            && event_widget->parent ()
            && ! event_widget->parent ()->parent ()
            && event_widget->parent ()->inherits ("QListBox"));
}

bool QTestControl::isToplevelEventFromUser(QObject* receiver)
{
    // Only user generated events should be recorded, so try to
    // find out if the Move or Resize event could not have been
    // generated by the human in front of the screen...

    if ( ! receiver->isWidgetType ())
        return FALSE;        // only widgets can be placed by user...

    QWidget* w = (QWidget*)receiver;
    // if ( ! w->isVisible ())  <-- defies portability
    //     return FALSE;        

    if ( ! w->isTopLevel ())
        return FALSE;

    if (w->isPopup ())
        return FALSE;        // popups cannot be placed by user
    
    //if ( ! ((QWidget*)receiver)->testWFlags (Qt::WStyle_NormalBorder | Qt::WStyle_DialogBorder))
    //    break;        // widgets without border cannot be placed by user (only some wm allow this)

    return TRUE;
}

void QTestControl::dumpPixmapToFile (QWidget& widget, QString& file_basename)
{
    assert (&widget);
    assert (&file_basename);

    QString filename = file_basename + ".png";
    QPixmap pm = QPixmap::grabWindow (widget.winId (), 0, 0, widget.width (), widget.height ());
    if ( ! pm.isNull ()) {

        pm.save (filename, "PNG");
        QString s;
	    s.sprintf("Wrote dump of widget %s to file %s",widget.name(),filename.latin1());

        QRemoteMessage S("PNG",s);
	    S.send(m_socket);
    }
}

bool QTestControl::hasName ( const QObject* item )
{
    if ( item == 0 )
	return FALSE;

    if ( !item->name() || strcmp (item->name(), "unnamed") == 0) {
	if (!item->isWidgetType())
	    return FALSE;
	else 
	    return ((QWidget*)item)->alias() != "";
    }

    return TRUE;
}

// getFullName() creates a name that is a concatenation of all parents' names. This is unique
// as long as you ensure that sibling widgets have unique names.
QCString QTestControl::getFullName( const QObject* item )
{
    if (item == 0)
	return "";

    QString name;

    if (item->isWidgetType() && !((QWidget*)item)->alias().isNull() && ((QWidget*)item)->alias() != "") {
	name = ((QWidget*)item)->alias();
    } else
	name = item->name();
    if (name.isNull())
	name = "null";

    QCString retValue;

//    if (isTopLevelItem(item))
    if (item->parent() == 0)
	retValue = name;
    else
	retValue = getFullName(item->parent()) + nameDelimiterChar + name.latin1();

    if (!hasName(item)) {

	QRemoteMessage S("WRN","Unnamed instance '" + retValue + "' of class '" + item->className() + "'");
	S.send(m_socket);
    }
//qDebug("getFullName: " + retValue);

    return retValue;
}

// verifyUniqueName() returns the same name as getFullName() but also verifies that there are
// no siblings with exactly the same name.
bool QTestControl::verifyUniqueName( const QObject* item, QCString &widgetName )
{
    bool retValue = TRUE;
    widgetName = getFullName(item);

    QObject* parent = item->parent();
    if (parent) {

	QObjectList *children = parent->queryList(0,item->name(),FALSE,FALSE);
	if (children) {
	    if (children->count() > 1) {

		// some workarounds to solve internal name problems...
		QString testName = item->name();
		if (testName == "qt_dockwidget_internal") {
//QMessageBox::warning(0,"Hi","Hi","Ok");

		    bool bad_class = FALSE;
		    int popup_count = 0;
		    int extension_count = 0;
		    int resize_count = 0;
		    int title_count = 0;
		    int handle_count = 0;
		    int alias_count = 0;
		    QWidget *w;
		    uint i;
		    for (i=0; i < children->count(); i++) {
			w = (QWidget*)children->at(i);
			if (w->alias() != "") {
			    alias_count++;
			    continue;
			}
			QString tmp = w->className();
			if (tmp == "QDockWindowTitleBar") {
			    title_count++;
			    w->setAlias("qt_titlebar");
			} else if (tmp == "QDockWindowHandle") {
			    switch (handle_count)
			    {
				case 0: w->setAlias("qt_horz_handle"); break;
				case 1: w->setAlias("qt_vert_handle"); break;
			    }
			    handle_count++;

			} else if (tmp == "QDockWindowResizeHandle") {
			    switch (resize_count)
			    {
				case 0: w->setAlias("qt_resize_top"); break;
				case 1: w->setAlias("qt_resize_left"); break;
				case 2: w->setAlias("qt_resize_right"); break;
				case 3: w->setAlias("qt_resize_bottom"); break;
			    }
			    resize_count++;

			} else if (tmp == "QToolBarExtensionWidget") {
			    extension_count++;
			    w->setAlias("qt_extension_widget");
			} else if (tmp == "QPopupMenu") {
			    popup_count++;
			    w->setAlias("qt_popup_menu");
			} else {
			    bad_class = TRUE;
			}
		    }

		    if (!bad_class) {
			if (alias_count == 9 ||
			   (popup_count == 1 && 
			    extension_count == 1 && 
			    resize_count == 4 && 
			    handle_count == 2 &&
			    title_count == 1
			   ) )
			return TRUE;
		    }

		    // turn back the clock so we get a proper error message from the children
		    for (i=0; i < children->count(); i++) {
			w = (QWidget*)children->at(i);
			w->setAlias("");
		    }
		}


		// check if we have issued a warning for this objectname before.
		// if we have, exit without sending another warning.
		QStringList::Iterator it = multiple_instances.begin();
		while (it != multiple_instances.end()) {
		    if (strcmp(*it,widgetName) == 0) {
			retValue = FALSE;
			break;
		    }
		    it++;
		}

		if (retValue) {
		    // add the name to the list...
		    multiple_instances.append(widgetName);

		    // and send a warning/error...
		    QRemoteMessage S("ERR","Multiple instances named '" + widgetName + "' exist.");
		    S.send(m_socket);

		    for (uint i=0; i < children->count(); i++) {
			QString tmp = "  of class type '";
			tmp+= children->at(i)->className();
			assert(children->at(i)->parent() == parent);
			tmp+= "'.";
			QRemoteMessage S("ERR",tmp);
			S.send(m_socket);
		    }

		    retValue = FALSE;
		}
	    }
	    delete children;
	}
    }

    return retValue;
}

bool QTestControl::doesWidgetExistRightNow( QObject* receiver )
{
    QCString widget_name = getFullName( receiver );

    widget_name += nameDelimiterChar;
    QString missing_name;
    QObject* event_widget = findAppWidgetByName( widget_name, missing_name, FALSE );
    return (event_widget != 0);
}

bool QTestControl::isWidgetAListBoxArea( const QObject* event_widget )
{
    assert (event_widget);
    if ( ! event_widget)
        return FALSE;

    return ((::strcmp(event_widget->name(), "qt_viewport") == 0)
            && event_widget->isA("QWidget")
            && event_widget->parent()
            && event_widget->parent()->parent()
            && event_widget->parent()->inherits("QListBox"));
}

bool QTestControl::isTopLevelItem(const QObject* item)
{
    return (item->isWidgetType() && ((QWidget*)item)->isTopLevel ());
}

bool QTestControl::receiverIsAccessible(QObject *receiver)
{
    assert(receiver);

    if (!receiver->parent()) {
	if (isTopLevelItem(receiver)) {

	    QObject* widget = findWidget((char*)receiver->name());
	    if (widget == 0) {
		return FALSE;
	    }
	}
    }

    return TRUE;
}

QObject* QTestControl::findWidget( const QCString &name )
{
    QString missing_name;
    QObject *widget = findAppWidgetByName(name, missing_name, TRUE);

    if (!widget)
	widget = findGlobalWidget(name);

    return widget;
}

QObject* QTestControl::findAppWidgetByName(const QCString &name, QString &missing_name, bool allow_invisible_widgets)
{
    // casting a QWidgetList to a QObjectList is safe as long as QWidget inherits QObject
    QObjectList *search_list = (QObjectList*)(QApplication::topLevelWidgets());  
    QObject *widget = findWidgetByName(name, search_list, missing_name, allow_invisible_widgets);
    delete search_list;
//    QObjectList *search_list = (QObjectList*)(QApplication::objectTrees());
//    QObject *widget = findWidgetByName(name, search_list, missing_name, TRUE);

    return widget;
}

QObject* QTestControl::findGlobalWidget( const QCString &name )
{
    QString missing_name;
    QObjectList *search_list = (QObjectList*)(QApplication::objectTrees());
    QObject *widget = findWidgetByName(name, search_list, missing_name, TRUE);

    return widget;
}

QObject* QTestControl::findWidgetByName( const QCString &name, const QObjectList* search_list, QString &missing_name, bool allow_invisible_widgets )
{
    bool last = FALSE;
    QObject* result = 0;
    if (search_list) {

	QCString base_name;
	int pos = name.find(nameDelimiterChar);
	if (pos > 0)
	    base_name = name.left(pos);
	else {
	    base_name = name;
	    last = TRUE;
	}

	QCString cur_name;
        QObjectListIt it (*search_list);
	QObject *o;
        while (it.current() && ! result) {

	    o = it.current();
	    if (o->isWidgetType() && ((QWidget*)o)->alias() != "")
		cur_name = ((QWidget*)o)->alias();
	    else
		cur_name = o->name();
	    if ((base_name == cur_name) &&
		    (allow_invisible_widgets || 
		 !o->isWidgetType() || 
		 ((QWidget*)o)->isVisible())) {

		    QCString nextname = name.mid(pos+1);     // next name starts after delimiter
		    if (nextname != "" && !last) {            // if there is another name...
			result = findWidgetByName(nextname, o->children(), missing_name, allow_invisible_widgets);
		    } else {
			if (o->isWidgetType())
			    result = it.current();
		    }
            }
            ++it;
        }
        if ( ! result && missing_name.isNull())
            missing_name = base_name;
    }
    return result;
}

void QTestControl::addWidgetDef( const QString &def )
{
    QCString  widget_name;
    int      widget_index;
    if (!widgetDefs.appendWidgetDef(def, widget_name, widget_index))
	return;

    for (QTestScalingInfo* si = scalingInfo.first(); si; si = scalingInfo.next())
    {
        if ((si->regExp()) && 
			(si->regExp()->isValid()) && 
			(si->regExp()->search(widget_name, 0) >= 0))
        {
            widget_scale_dict_.insert(widget_index, si);
        }
    }
}

QTestControl::QTestScalingInfo::QTestScalingInfo(QByteArray *params, ScaleMode sm)
: scale_mode_ (sm)
, scale_filter_ (0)
{
    QDataStream ts (*params, IO_ReadOnly);
    QString regexp;
    QChar   asterisk_dummy;
//    ts >> asterisk_dummy >> scale_h_value_ >> scale_v_value_ >> regexp;
    assert(FALSE);
    scale_filter_ = new QRegExp(regexp);
}

QTestControl::QTestScalingInfo::~QTestScalingInfo ()
{
/*
    if (widget_scale_dict_)
    {
        QIntDictIterator<QTestControl::QTestScalingInfo>  it (*widget_scale_dict_);
        while (it.current ())
        {
            if (it.current () == this)
                widget_scale_dict_.remove (it.currentKey ());

            ++it;
        }
    }
*/
    delete scale_filter_;
}

QMouseEvent* QTestControl::readMouseEvent(QEvent::Type event_id, QTextStream& replay_stream, QObject* &event_widget, int widget_id)
{
    int x;
    int y;
    int button;
    int state;
    replay_stream >> x >> y >> button >> state;

    const QTestScalingInfo* scale_info = widget_scale_dict_.find (widget_id);
    if (scale_info)
    {
        if (scale_info->scaleMode () == QTestScalingInfo::TopLeftScaling)
        {
            x *= scale_info->scaleHValue ();
            y *= scale_info->scaleVValue ();
        }
        else
        {
            assert (scale_info->scaleMode () == QTestScalingInfo::CenterScaling);
            if (event_widget->isWidgetType ())
            {
                const double h_center_offset = 0.5 * ((QWidget*)event_widget)->width ()  - x;
                const double v_center_offset = 0.5 * ((QWidget*)event_widget)->height () - y;
                if (h_center_offset)
                    x *= scale_info->scaleHValue () * h_center_offset;

                if (v_center_offset)
                    y *= scale_info->scaleVValue () * v_center_offset;
            }
        }
    }
    QPoint mouse_pos (x, y);

    int menu_item;
    replay_stream >> menu_item;

    QString menu_text;
    replay_stream >> menu_text;//menu_text = replay_stream.readLine ().mid (1);

    if ((menu_item >= 0) && (y >= 0))
    {
        if (event_widget->inherits ("QMenuBar"))
        {
            if (!((QTestMenuBar*)event_widget)->index2MousePos(menu_item, mouse_pos)) {

		QString s;
		s.sprintf("Menubar item nr %d does not exist", menu_item);

		QRemoteMessage S("ERR",s);
		S.send(m_socket);
	    }
        }
        else if (event_widget->inherits ("QPopupMenu"))
        {
            QTestPopupMenu* pm = (QTestPopupMenu*)event_widget;
            if (menu_text.isEmpty ())
                if (!pm->index2MousePos(menu_item,mouse_pos)) {

		    QString s;
		    s.sprintf("A popupmenu item nr %d does not exist", menu_item);

		    QRemoteMessage S("ERR",s);
		    S.send(m_socket);
		}
            else {
                if (!pm->index2MousePos(menu_text,mouse_pos)) {

		    QString s;
		    s.sprintf("A popupmenu item that matches '%s' does not exist",menu_text.latin1());

		    QRemoteMessage S("ERR",s);
		    S.send(m_socket);
		}
	    }
        }
        else if (event_widget->inherits ("QListBox"))
        {
            QTestListBox* lbox = (QTestListBox*)event_widget;
            if (menu_text.isEmpty ())
                lbox->index2MousePos(menu_item, mouse_pos);
            else
                if (!lbox->index2MousePos(menu_text, menu_item, mouse_pos)) {

		    QString s;
		    s.sprintf("No item with text '%s' found", menu_text.latin1());

		    QRemoteMessage S("ERR",s);
		    S.send(m_socket);
		}
        } else if (isWidgetAListBoxArea (event_widget) || isWidgetAComboBoxPanel (event_widget)) {

            QTestListBox* lbox = ((QTestListBox*)(event_widget->parent ()));
            if (menu_text.isEmpty ())
                lbox->index2MousePos(menu_item, mouse_pos);
            else
                if (!lbox->index2MousePos(menu_text, menu_item, mouse_pos)) {

		    QString s;
		    s.sprintf("No item with text '%s' found", menu_text.latin1());

		    QRemoteMessage S("ERR",s);
		    S.send(m_socket);
		}
        }
    }
    else if (menu_item == -2)
    {
        QCString  full_listviewitem_name;
        full_listviewitem_name = menu_text.mid (1).latin1 ();
        if (event_widget->parent ()
            && event_widget->parent ()->inherits ("QListView"))
        {
            QListViewItem* item = findListViewItemByName (((QListView*)(event_widget->parent ()))->firstChild (),
                                                          full_listviewitem_name);
            if (item)
            {
                item->listView ()->ensureItemVisible (item);
                mouse_pos.setY (item->listView ()->itemRect (item).y () + 3);
            }
        }
    }
    /*
    else if (event_widget->isWidgetType () 
             && ((QWidget*)event_widget)->isPopup ()
             && ! ((QWidget*)event_widget)->frameGeometry ().contains (((QWidget*)event_widget)->pos () + mouse_pos)
            )
    {
        QPoint new_xy = ((QWidget*)event_widget)->pos () + mouse_pos;
        QWidget* new_event_widget = QApplication::widgetAt (new_xy, TRUE);
        if (new_event_widget)
        {
            //event_widget = new_event_widget;
            //mouse_pos = new_event_widget->mapFromGlobal (new_xy);
        }
    }*/

    assert (event_widget->isWidgetType ());
    QPoint global_mouse_pos = ((QWidget*)event_widget)->mapToGlobal (mouse_pos);
    QCursor::setPos (global_mouse_pos);

    return new QMouseEvent (event_id, mouse_pos, global_mouse_pos, button, state);
}

QMoveEvent*  QTestControl::readMoveEvent (QTextStream& replay_stream, QObject* widget)
{
    int x;
    int y;
    int oldX;
    int oldY;
    replay_stream >> x >> y >> oldX >> oldY;
    if (widget
        && widget->isWidgetType ()
        && ((QWidget*)widget)->testWFlags (Qt::WStyle_NormalBorder | Qt::WStyle_DialogBorder)
       )
    {
        ((QWidget*)widget)->move (x, y);
    }
    return new QMoveEvent (QPoint (x, y), QPoint (oldX, oldY));
}

QResizeEvent*  QTestControl::readResizeEvent (QTextStream& replay_stream, QObject* widget)
{
    int x;
    int y;
    int oldX;
    int oldY;
    replay_stream >> x >> y >> oldX >> oldY;
    if (widget
        && widget->isWidgetType ()
        && ((QWidget*)widget)->testWFlags (Qt::WStyle_NormalBorder | Qt::WStyle_DialogBorder)
       )
    {
        ((QWidget*)widget)->resize (x, y);
    }
    return new QResizeEvent (QSize (x, y), QSize (oldX, oldY));
}

QKeyEvent* QTestControl::readKeyEvent(QEvent::Type event_id, QTextStream& replay_stream)
{
    int key;
    int ascii;
    int state;
    int      autorep = FALSE;
    ushort   count   = 1;
    QString  text    = QString::null;
    replay_stream >> key >> ascii >> state;
    int text_length;
    replay_stream >> autorep >> count >> text_length;
    while (text_length > 0)
    {
qDebug("QTestControl::readKeyEvent()...");
        ushort new_char;
        replay_stream >> new_char;
        text += QChar (new_char);
        --text_length;
    }

    if (event_id == QEvent::KeyPress)
    {
        if ((state & (Qt::ShiftButton | Qt::ControlButton)) == (Qt::ShiftButton | Qt::ControlButton))
        {
            QWidget* dump_widget = 0;
            if (key == windowDumpKey)
            {
                dump_widget = QApplication::widgetAt (QCursor::pos (), FALSE);
                dump_widget = dump_widget->topLevelWidget ();  // work around bug in Qt 1.x & 2.0
            }
            else if (key == widgetDumpKey)
                dump_widget = QApplication::widgetAt (QCursor::pos (), TRUE);

            if (dump_widget)
            {
                QString dump_filename = "_pxmdump_";
                int ext_pos = dump_filename.findRev ('.');
                if (ext_pos > 0)
                    dump_filename.truncate (ext_pos);
                dump_filename += "_rep_";
                dump_filename += QString ().setNum (playFileCount);
                dumpPixmapToFile (*dump_widget, dump_filename);
                ++playFileCount;              
            }
        }
    }
    return new QKeyEvent (event_id, key, ascii, state, text, autorep, count);
}

QListViewItem* QTestControl::findListViewItemByName (QListViewItem* first_child, QCString& name)
{
    const int delimiter_pos = name.find (nameDelimiterChar);
    if (delimiter_pos >= 0)
    {
        QCString depth_name = name.left (delimiter_pos);
        name = name.mid (delimiter_pos+1, name.length ());
        QListViewItem* current_item = first_child;
        while (current_item)
        {
qDebug("QTestControl::findListViewItemByName()...");
            if (depth_name == current_item->text (0).latin1 ())
            {
                if (name.isEmpty ())
                    return current_item;
                else
                    return findListViewItemByName (current_item->firstChild (), name);
            }
            current_item = current_item->nextSibling ();
        }
    }
    return 0;
}

/*!
   \internal
	Searches for a replymessage \a msg with the specified \a msgId.
	A reply message is a reply received from the remote controller and is usually 
	the answer to a question send before to the remote controller using sendObject().
	Returns TRUE if the requested reply was received.
*/

bool QTestControl::findMessage(uint msgId, QRemoteMessage *&msg)
{
	for (uint i=0; i<m_replyList.count(); i++) {

		msg = m_replyList.at(i);
		if ((msg) && (msg->messageId() == msgId)) {
		
			m_replyList.remove(i);
			return TRUE;
		}
	}

	return FALSE;
}

/*!
   \internal
    Posts (e.g. non blocking) an \a event, \a message and \a data to the remote 
    controller (host).
*/

void QTestControl::postObject(const QString &event, const QString &message, const QByteArray *data)
{
    if ((m_socket->state() == QSocket::Connected) &&
	(m_socket->socketDevice()->error() == QSocketDevice::NoError)) {

	QRemoteMessage S( event, message, data );
	S.send( m_socket );
    } else {
	qDebug( "socket error" );
    }
}

/*!
   \internal
    Sends (e.g. blocking) a \a event, \a message and \a data to the remote controller 
    (host) and waits for a \a result. You can use \a timeout to specify the max wait time
    for the reply. If \a timeout == -1 the function waits forever.
    The function returns TRUE if a reply is received before a timeout has elapsed.
*/

bool QTestControl::sendObject(const QString &event, const QString &message, const QByteArray *data, QString &result, int timeout)
{
    Q_UNUSED( timeout );
    if ((m_socket->state() == QSocket::Connected) &&
	(m_socket->socketDevice()->error() == QSocketDevice::NoError)) {

	QRemoteMessage out( event, message, data );
	out.send( m_socket );

	QRemoteMessage *msg = 0;
	while (!findMessage(out.messageId(), msg) && 
	       (m_socket->state() == QSocket::Connected) &&
	       (m_socket->socketDevice()->error() == QSocketDevice::NoError)) {

		if (m_socket->waitForMore(1000) > 0) {

		    onData();
		} else {

		    qApp->processEvents(1000);
		}
	    }

	    if (msg != 0) {

		result = msg->result();
		delete msg;
		return TRUE;
	    } else 
		return FALSE;
    } else {
	return FALSE;
    }
}

/*!
   \internal
    Sends the debug statement \a msg to the remote control system.
    So instead of presenting a debug statement on your local machine you'll get the 
    output on the other machine.
*/

void QTestControl::rDebug(const QString msg)
{
#ifdef DEBUG
    QRemoteMessage S("Debug",msg);
    S.send(m_socket);
#endif
}
