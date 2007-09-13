/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QTreeWidget)
QT_DECLARE_CLASS(QTreeWidgetItem)
QT_DECLARE_CLASS(QPushButton)

class RSSListing : public QWidget
{
    Q_OBJECT
public:
    RSSListing(QWidget *widget = 0);

public slots:
    void fetch();
    void finished(int id, bool error);
    void readData(const QHttpResponseHeader &);
    void itemActivated(QTreeWidgetItem * item);

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
    QPushButton *abortButton;
    QPushButton *fetchButton;
};

#endif

