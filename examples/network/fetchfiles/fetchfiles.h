/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef FETCHFILES_H
#define FETCHFILES_H

#include <qobject.h>
#include <qurloperator.h>
#include <qstringlist.h>

#include "fetchfiles.h"

class QUrlOperator;

class FetchFiles : public QObject
{
    Q_OBJECT

public:
    FetchFiles( QObject *parent = 0, const char *name = 0 );
    void fetch( const QString& path, const QString& dest );

public slots:
    void stop();

signals:
    void start();
    void startFile( const QString& path );
    void finishedFile( const QString& path );
    void finished();
    void error();

private slots:
    void opStart( QNetworkOperation* op );
    void opFinished( QNetworkOperation* op );
    void opNewChildren( const QValueList<QUrlInfo>& i, QNetworkOperation* op );

private:
    QUrlOperator urlOp;
    QStringList files;
    QStringList::iterator currFile;
    QString destFolder;
};

#endif //FETCHFILES_H