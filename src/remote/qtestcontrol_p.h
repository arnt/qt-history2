/****************************************************************************
** $Id: $
**
** Definition of QTestControl class
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

#ifndef QTESTCONTROL
#define QTESTCONTROL

#include <time.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qsocket.h>
#include <private/qremotemessage_p.h>
#include <qremotecontrol.h>
#include "widgetdefs_p.h"
#include <quuid.h>

static const char regexp_magic[] = "@@RegExp@@:";

class QTestControl : public QRemoteControl
{
    Q_OBJECT

public:
    QTestControl();
    virtual ~QTestControl();

    // Implementation of QRemoteControl
    virtual void open( const QString& hostname, int port );
    virtual void close();
    virtual bool isOpen();
    virtual bool handleNotification( QObject * receiver, QEvent * e );
    virtual void postObject( const QString &event, const QString &message ) { postObject( event, message, 0); };
    virtual void postObject( const QString &event, const QString &message, const QByteArray *data = 0 );
    virtual bool sendObject( const QString &event, const QString &message, const QByteArray *data, QString &result, int timeout = -1 );
    virtual void setRemoteClient( QRemoteClient *receiver );

protected:
    void startRecording();
    void stopRecording();

    void startReplay();
    void stopReplay();
    
    bool findMessage( uint msgId, QRemoteMessage *&msg );
    QRemoteClient *m_remoteClient;

    // use for debugging only (of course)!
    void rDebug(const QString msg);

public slots:
    void hostFound();
    void hostConnected();
    void error(int error);
    void onData();
    void hostClosed();
    void onCommand();

private:
    static QUuid cid;
    static unsigned long ref;
    QPtrList<QRemoteMessage>	m_replyList;
    QPtrList<QRemoteMessage>	m_cmdList;
    QRemoteMessage		*curMessage;
    enum			{recordVersion = 12};
    int				replayVersion;
    int				replay_file_qt_version_;

    long			recFileCount;
    long			playFileCount;
    QSocket			*m_socket;

    bool			is_recording;
    bool			is_replaying;
    int				windowDumpKey;
    int				widgetDumpKey;
    char			nameDelimiterChar;
    QCString			nameDelimiterString;
    QCString			anonymous_magic_;
    time_t			replay_file_date_;  // in seconds since Jan. 1st, 1970 UTC
    bool			isSimulatedEvent;
    QStringList			multiple_instances;

    WidgetDefs	                widgetDefs;

    class QTestScalingInfo
    {
    public:
        enum ScaleMode {TopLeftScaling, CenterScaling};

        QTestScalingInfo (QByteArray *params, ScaleMode sm);
        ~QTestScalingInfo ();
        ScaleMode  scaleMode () const    { return scale_mode_; }
        double     scaleHValue () const  { return scale_h_value_; }
        double     scaleVValue () const  { return scale_v_value_; }
        const QRegExp* regExp () const   { return scale_filter_; }

    protected:
        QRegExp*     scale_filter_;
        double       scale_h_value_;
        double       scale_v_value_;
        ScaleMode    scale_mode_;
    };
    friend class QTestScalingInfo;
    QPtrList<QTestScalingInfo>		scalingInfo;
    QIntDict<QTestScalingInfo>	widget_scale_dict_;

private:
    void recordEvent( QObject* receiver, QEvent* e );
    bool simulateEvent( QRemoteMessage *msg );

    void getEventLine( QKeyEvent* e, QString& event_line );
    void getEventLine( QMouseEvent* e, const QObject* event_item, QString& event_line );
    bool getEventLine( QMoveEvent* e, const QPoint& widget_pos, QString& event_line );
    bool getEventLine( QResizeEvent* e, QString& event_line );

    QMouseEvent* readMouseEvent( QEvent::Type event_id, QTextStream& replay_stream, QObject* &event_item, int widget_id );
    QKeyEvent* readKeyEvent( QEvent::Type event_id, QTextStream& replay_stream );
    QMoveEvent* readMoveEvent( QTextStream& replay_stream, QObject* widget );
    QResizeEvent* readResizeEvent( QTextStream& replay_stream, QObject* widget );

    bool isWidgetAComboBoxPanel( const QObject* event_widget );
    bool isToplevelEventFromUser( QObject* receiver );
    bool isWidgetAListBoxArea( const QObject* event_widget );
    bool isTopLevelItem( const QObject* item );
    bool doesWidgetExistRightNow( QObject* widget );
    bool mustWriteEvent( QEvent* e, QObject* receiver );
    QListViewItem* findListViewItemByName( QListViewItem* first_child, QCString& name );

    bool hasName( const QObject* item );
    QCString getFullName( const QObject* item );
    bool verifyUniqueName( const QObject *item, QCString &name );

    QObject* findAppWidgetByName( const QCString &name, QString &missing_name, bool allow_invisible_widgets);
    QObject* findWidgetByName( const QCString &name, const QObjectList *search_list, QString& missing_name, bool allow_invisible_widgets);
    QObject* findGlobalWidget( const QCString &name );
    QObject* findWidget( const QCString &name );
    bool receiverIsAccessible( QObject *receiver );

    void dumpPixmapToFile( QWidget& widget, QString& file_basename );
    bool appendWidget( const QCString &name, int widget_index );
    void updateWidgetDefs( const QObjectList* list );
    void addWidgetDef( const QString &def );
    void sendWidgetDefs();

    void notify( QObject* event_widget, QEvent* event );
    void handleCommand( QRemoteMessage *msg );
};

#endif
