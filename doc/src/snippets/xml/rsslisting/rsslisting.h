/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef RSSLISTING_H
#define RSSLISTING_H

#include <qhttp.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qxml.h>

#include "handler.h"

class RSSListing : public QWidget
{
    Q_OBJECT
public:
    RSSListing(QWidget *widget = 0, const char *name = 0, WFlags flags = 0);

public slots:
    void addItem(QString &title, QString &link);
    void fetch();
    void finished(int id, bool error);
    void readData(const QHttpResponseHeader &);

private:
    QXmlSimpleReader xmlReader;
    QXmlInputSource xmlInput;
    Handler *handler;

    bool newInformation;

    QHttp http;
    int connectionId;

    QLineEdit *lineEdit;
    QListView *listView;
    QListViewItem *lastItemCreated;
    QPushButton *abortButton;
    QPushButton *fetchButton;
};

#endif

