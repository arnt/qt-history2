/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SMTP_H
#define SMTP_H

#include <qobject.h>
#include <qstring.h>

class QSocket;

class Smtp : public QObject
{
    Q_OBJECT
    
public:
    Smtp( const QString &server_, int port_,
	  const QString &from_, const QString &to_, const QString &subject_,
	  const QString &cc_, const QString &bcc_, const QString &message_ );
    
signals:
    void finished();
    
private slots:
    void readyRead();
    void connected();
    void deleteMe();
    
private:
    enum State {
	Init = 0,
	Mail, 
	Rcpt, 
	Data, 
	From, 
	To, 
	Subject, 
	Cc, 
	Bcc,
	Message,
	DataEnd, 
	Quit,
	Close
    };
    
    QString server, from, to, subject, cc, bcc, message;
    int port;
    QSocket *socket;
    int state;
    
};

#endif
