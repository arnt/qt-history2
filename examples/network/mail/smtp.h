/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SMTP_H
#define SMTP_H

#include <qobject.h>
#include <qstring.h>

class QSocket;
class QTextStream;
class Q3Dns;

class Smtp : public QObject
{
    Q_OBJECT

public:
    Smtp( const QString &from, const QString &to,
	  const QString &subject, const QString &body );
    ~Smtp();

signals:
    void status( const QString & );

private slots:
    void dnsLookupHelper();
    void readyRead();
    void connected();

private:
    enum State {
	Init,
	Mail,
	Rcpt,
	Data,
	Body,
	Quit,
	Close
    };

    QString message;
    QString from;
    QString rcpt;
    QSocket *socket;
    QTextStream * t;
    int state;
    QString response;
    Q3Dns * mxLookup;
};

#endif
