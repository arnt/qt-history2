#include <QtGui>
#include <QtNetwork>

#include "ftp.h"
#include "icons.cpp"

Ftp::Ftp(QWidget *parent)
    : QDialog(parent)
{
    ftpServerLabel = new QLabel(tr("Ftp &server:"), this);
    ftpServerLineEdit = new QLineEdit("ftp.trolltech.com", this);
    ftpServerLabel->setBuddy(ftpServerLineEdit);

    statusLabel = new QLabel(tr("Please enter the name of an FTP server."), this);

    fileList = new QListWidget(this);

    connectButton = new QPushButton(tr("Connect"), this);
    connectButton->setDefault(true);

    downloadButton = new QPushButton(tr("Download"), this);
    downloadButton->setEnabled(false);

    cdToParentButton = new QPushButton(QIconSet(QPixmap(cdtoparent_xpm)),
                                       QString::null, this);
    cdToParentButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"), this);

    ftp = new QFtp(this);

    progressDialog = new QProgressDialog(this);

    connect(ftpServerLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableConnectButton()));
    connect(fileList, SIGNAL(doubleClicked(QListWidgetItem *, int)),
            this, SLOT(processItem(QListWidgetItem *)));
    connect(fileList, SIGNAL(returnPressed(QListWidgetItem *)),
            this, SLOT(processItem(QListWidgetItem *)));
    connect(fileList, SIGNAL(currentChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(enableDownloadButton(QListWidgetItem *)));
    connect(ftp, SIGNAL(commandFinished(int, bool)),
            this, SLOT(ftpCommandFinished(int, bool)));
    connect(ftp, SIGNAL(listInfo(const QUrlInfo &)),
            this, SLOT(addToList(const QUrlInfo &)));
    connect(ftp, SIGNAL(dataTransferProgress(Q_LLONG, Q_LLONG)),
            this, SLOT(updateDataTransferProgress(Q_LLONG, Q_LLONG)));
    connect(progressDialog, SIGNAL(canceled()),
            this, SLOT(cancelDownload()));
    connect(connectButton, SIGNAL(clicked()),
            this, SLOT(connectToFtpServer()));
    connect(cdToParentButton, SIGNAL(clicked()),
            this, SLOT(cdToParent()));
    connect(downloadButton, SIGNAL(clicked()),
            this, SLOT(downloadFile()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(ftpServerLabel);
    topLayout->addWidget(ftpServerLineEdit);
    topLayout->addStretch(1);
    topLayout->addWidget(cdToParentButton);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(fileList);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Ftp"));
}

void Ftp::enableConnectButton()
{
    connectButton->setEnabled(!ftpServerLineEdit->text().isEmpty());
}

void Ftp::connectToFtpServer()
{
    ftp->connectToHost(ftpServerLineEdit->text());
    ftp->list();
    statusLabel->setText(tr("Connecting to FTP server %1.")
                         .arg(ftpServerLineEdit->text()));
}

void Ftp::ftpCommandFinished(int, bool error)
{
    if (ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            QMessageBox::information(this, tr("Ftp"),
                                     tr("Unable to connect to the FTP server "
                                        "at %1. Please check that the host "
                                        "name is correct.")
                                     .arg(ftpServerLineEdit->text()));
            return;
        }

        statusLabel->setText(tr("Connected to %1.").arg(ftpServerLineEdit->text()));
        fileList->setFocus();
        connectButton->setEnabled(false);
        downloadButton->setDefault(true);
        return;
    }

    if (ftp->currentCommand() == QFtp::Get) {
        statusLabel->setText(tr("Downloaded %1.").arg(file->fileName()));
        file->close();
        delete file;
        file = 0;
    }

    if (ftp->currentCommand() == QFtp::List) {
        if (isDirectory.isEmpty()) {
            fileList->appendItem(tr("<empty>"));
            fileList->setEnabled(false);
        }
    }
}

void Ftp::addToList(const QUrlInfo &urlInfo)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(urlInfo.name());
    item->setIcon(QIconSet(QPixmap(urlInfo.isDir() ? dir_xpm : file_xpm)));

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    fileList->appendItem(item);
    if (!fileList->currentItem()) {
        fileList->setCurrentItem(fileList->item(0));
        fileList->setEnabled(true);
    }
}

void Ftp::processItem(QListWidgetItem *item)
{
    QString name = item->text();
    if (isDirectory.value(name)) {
        fileList->clear();
        isDirectory.clear();
        currentPath += "/" + name;
        ftp->cd(name);
        ftp->list();
        cdToParentButton->setEnabled(true);
        return;
    }
}

void Ftp::downloadFile()
{
    QString fileName = fileList->currentItem()->text();

    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("Ftp"),
                                 tr("You already have a file called %1.")
                                 .arg(fileName));
        return;
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Ftp"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
    }

    ftp->get(fileList->currentItem()->text(), file);

    progressDialog->setLabel(new QLabel(tr("Downloading %1").arg(fileName), this));
    progressDialog->show();
    downloadButton->setEnabled(false);
}

void Ftp::cancelDownload()
{
    file->remove();
    delete file;
    file = 0;
}

void Ftp::updateDataTransferProgress(Q_LLONG readBytes, Q_LLONG totalBytes)
{
    progressDialog->setTotalSteps(totalBytes);
    progressDialog->setProgress(readBytes);
}

void Ftp::cdToParent()
{
    fileList->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    ftp->cd(currentPath);
    ftp->list();

    if (currentPath.isEmpty())
        cdToParentButton->setEnabled(false);
}

void Ftp::enableDownloadButton(QListWidgetItem *current)
{
    downloadButton->setEnabled(!isDirectory.value(current->text()));
}
