/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#include <QHttp>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

class RSSListing : public QWidget
{
    Q_OBJECT
public:
    RSSListing(QWidget *widget = 0);

public slots:
    void fetch();
    void finished(int id, bool error);
    void readData(const QHttpResponseHeader &);

private:
    void parseXml();

    QXmlStreamReader xml;
    QString currentTag;
    QString linkString;
    QString titleString;

    QHttp http;
    int connectionId;

    QLineEdit *lineEdit;
    QTreeWidget *treeWidget;
    QTreeWidgetItem *lastItemCreated;
    QPushButton *abortButton;
    QPushButton *fetchButton;
};

#endif

