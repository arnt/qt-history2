/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FTPWINDOW_H
#define FTPWINDOW_H

#include <QDialog>
#include <QHash>

class QDialogButtonBox;
class QFile;
class QFtp;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressDialog;
class QPushButton;
class QUrlInfo;

class FtpWindow : public QDialog
{
    Q_OBJECT

public:
    FtpWindow(QWidget *parent = 0);

private slots:
    void connectOrDisconnect();
    void downloadFile();
    void cancelDownload();

    void ftpCommandFinished(int commandId, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void processItem(QListWidgetItem *item);
    void cdToParent();
    void updateDataTransferProgress(qint64 readBytes,
                                    qint64 totalBytes);
    void enableDownloadButton();

private:
    QLabel *ftpServerLabel;
    QLineEdit *ftpServerLineEdit;
    QLabel *statusLabel;
    QListWidget *fileList;
    QPushButton *cdToParentButton;
    QPushButton *connectButton;
    QPushButton *downloadButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QProgressDialog *progressDialog;

    QHash<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp;
    QFile *file;
};

#endif
