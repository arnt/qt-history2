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

#include <QtGui>
#include <QtNetwork>

#include "ftpwindow.h"

FtpWindow::FtpWindow(QWidget *parent)
    : QDialog(parent)
{
    ftpServerLabel = new QLabel(tr("Ftp &server:"));
    ftpServerLineEdit = new QLineEdit("ftp.trolltech.com");
    ftpServerLabel->setBuddy(ftpServerLineEdit);

    statusLabel = new QLabel(tr("Please enter the name of an FTP server."));

    fileList = new QListWidget;

    connectButton = new QPushButton(tr("Connect"));
    connectButton->setDefault(true);

    downloadButton = new QPushButton(tr("Download"));
    downloadButton->setEnabled(false);

    cdToParentButton = new QPushButton;
    cdToParentButton->setIcon(QPixmap(":/images/cdtoparent.png"));
    cdToParentButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    ftp = new QFtp(this);

    progressDialog = new QProgressDialog(this);

    connect(ftpServerLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableConnectButton()));
    connect(fileList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(processItem(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemEntered(QListWidgetItem *)),
            this, SLOT(processItem(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(enableDownloadButton()));
    connect(ftp, SIGNAL(commandFinished(int, bool)),
            this, SLOT(ftpCommandFinished(int, bool)));
    connect(ftp, SIGNAL(listInfo(const QUrlInfo &)),
            this, SLOT(addToList(const QUrlInfo &)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
            this, SLOT(updateDataTransferProgress(qint64, qint64)));
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectToFtpServer()));
    connect(cdToParentButton, SIGNAL(clicked()), this, SLOT(cdToParent()));
    connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(ftpServerLabel);
    topLayout->addWidget(ftpServerLineEdit);
    topLayout->addWidget(cdToParentButton);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(fileList);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("FTP"));
}

void FtpWindow::connectToFtpServer()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ftp->connectToHost(ftpServerLineEdit->text());
    ftp->login();
    ftp->list();
    statusLabel->setText(tr("Connecting to FTP server %1...")
                         .arg(ftpServerLineEdit->text()));
}

void FtpWindow::downloadFile()
{
    QString fileName = fileList->currentItem()->text();

    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("There already exists a file called %1 in "
                                    "the current directory.")
                                 .arg(fileName));
        return;
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        return;
    }

    ftp->get(fileList->currentItem()->text(), file);

    progressDialog->setLabelText(tr("Downloading %1...").arg(fileName));
    progressDialog->show();
    downloadButton->setEnabled(false);
}

void FtpWindow::cancelDownload()
{
    ftp->abort();
}

void FtpWindow::ftpCommandFinished(int, bool error)
{
    if (ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server "
                                        "at %1. Please check that the host "
                                        "name is correct.")
                                     .arg(ftpServerLineEdit->text()));
            return;
        }

        statusLabel->setText(tr("Logged onto %1.")
                             .arg(ftpServerLineEdit->text()));
        fileList->setFocus();
        connectButton->setEnabled(false);
        downloadButton->setDefault(true);
        return;
    }

    if (ftp->currentCommand() == QFtp::Get) {
        QApplication::restoreOverrideCursor();
        if (error) {
            statusLabel->setText(tr("Canceled download of %1.")
                                 .arg(file->fileName()));
            file->close();
            file->remove();
            enableDownloadButton();
        } else {
            statusLabel->setText(tr("Downloaded %1 to current directory.")
                                 .arg(file->fileName()));
            file->close();
        }
        delete file;
    } else if (ftp->currentCommand() == QFtp::List) {
        QApplication::restoreOverrideCursor();
        if (isDirectory.isEmpty()) {
            fileList->addItem(tr("<empty>"));
            fileList->setEnabled(false);
        }
    }
}

void FtpWindow::addToList(const QUrlInfo &urlInfo)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(urlInfo.name());
    QPixmap pixmap(urlInfo.isDir() ? ":/images/dir.png" : ":/images/file.png");
    item->setIcon(pixmap);

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    fileList->addItem(item);
    if (!fileList->currentItem()) {
        fileList->setCurrentItem(fileList->item(0));
        fileList->setEnabled(true);
    }
}

void FtpWindow::processItem(QListWidgetItem *item)
{
    QString name = item->text();
    if (isDirectory.value(name)) {
        fileList->clear();
        isDirectory.clear();
        currentPath += "/" + name;
        ftp->cd(name);
        ftp->list();
        cdToParentButton->setEnabled(true);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        return;
    }
}

void FtpWindow::cdToParent()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    fileList->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    if (currentPath.isEmpty()) {
        cdToParentButton->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(currentPath);
    }
    ftp->list();
}

void FtpWindow::updateDataTransferProgress(qint64 readBytes,
                                           qint64 totalBytes)
{
    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(readBytes);
}

void FtpWindow::enableConnectButton()
{
    connectButton->setEnabled(!ftpServerLineEdit->text().isEmpty());
}

void FtpWindow::enableDownloadButton()
{
    QListWidgetItem *current = fileList->currentItem();
    if (current) {
        QString currentFile = current->text();
        downloadButton->setEnabled(!isDirectory.value(currentFile));
    } else {
        downloadButton->setEnabled(false);
    }
}
