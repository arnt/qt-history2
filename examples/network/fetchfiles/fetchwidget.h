/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef FETCHWIDGET_H
#define FETCHWIDGET_H

#include "fetchwidgetbase.h"

class FetchFiles;

class FetchWidget : public FetchWidgetBase 
{
    Q_OBJECT

public:
    FetchWidget( QWidget *parent = 0, const char *name = 0 );

signals:
    void stop();

private slots:
    void getFtpDir();
    void fetch();
    void start();
    void startFile( const QString& path );
    void finishedFile( const QString& path );
    void error();
    void finished();

private:
    QString getSelectedDir();
    FetchFiles *ff;
};

#endif //FETCHWIDGET_H