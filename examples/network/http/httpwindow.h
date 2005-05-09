/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HTTPWINDOW_H
#define HTTPWINDOW_H

#include <QDialog>

class QFile;
class QHttp;
class QHttpResponseHeader;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;

class HttpWindow : public QDialog
{
    Q_OBJECT

public:
    HttpWindow(QWidget *parent = 0);

private slots:
    void downloadFile();
    void cancelDownload();
    void httpRequestFinished(int requestId, bool error);
    void readResponseHeader(const QHttpResponseHeader &responseHeader);
    void updateDataReadProgress(int bytesRead, int totalBytes);
    void enableDownloadButton();

private:
    QLabel *statusLabel;
    QLabel *urlLabel;
    QLineEdit *urlLineEdit;
    QProgressDialog *progressDialog;
    QPushButton *quitButton;
    QPushButton *downloadButton;

    QHttp *http;
    QFile *file;
    int httpGetId;
    bool httpRequestAborted;
};

#endif
