#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "http.h"

Http::Http(QWidget *parent)
    : QDialog(parent)
{
    urlLineEdit = new QLineEdit("http://www.trolltech.com/video/overview.rm",
                                this);

    urlLabel = new QLabel(tr("&Url:"), this);
    urlLabel->setBuddy(urlLineEdit);
    statusLabel = new QLabel(tr("Please enter the URL of "
                                "a file you want to download."),
                             this);

    quitButton = new QPushButton(tr("Quit"), this);
    downloadButton = new QPushButton(tr("Download"), this);
    downloadButton->setDefault(true);

    progressDialog = new QProgressDialog(this);

    http = new QHttp(this);

    connect(urlLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableDownloadButton()));
    connect(http, SIGNAL(requestFinished(int, bool)),
            this, SLOT(httpRequestFinished(int, bool)));
    connect(http, SIGNAL(dataReadProgress(int, int)),
            this, SLOT(updateDataReadProgress(int, int)));
    connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
            this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    connect(downloadButton, SIGNAL(clicked()), this, SLOT(download()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *urlLayout = new QHBoxLayout;
    urlLayout->addWidget(urlLabel);
    urlLayout->addWidget(urlLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(urlLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Http"));
    urlLineEdit->setFocus();
}

void Http::download()
{
    QUrl url(urlLineEdit->text());
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("Http"),
                                 tr("You already have a file called %1.")
                                 .arg(fileName));
        return;
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Http"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

    http->setHost(url.host(), url.port() != -1 ? url.port() : 80);
    if (!url.userName().isEmpty())
        http->setUser(url.userName(), url.password());

    httpRequestAborted = false;
    httpGetId = http->get(url.path(), file);

    progressDialog->setLabel(new QLabel(tr("Downloading %1.").arg(fileName),
                                        this));

    downloadButton->setEnabled(false);
}

void Http::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    http->abort();
}

void Http::httpRequestFinished(int id, bool error)
{
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        return;
    }

    if (id != httpGetId)
        return;

    file->close();

    if (error) {
        file->remove();
        QMessageBox::information(this, tr("QHttp"),
                                 tr("Download failed: %1")
                                 .arg(http->errorString()));
    } else {
        statusLabel->setText(tr("Download complete."));
    }

    downloadButton->setEnabled(true);
    delete file;
    file = 0;
}

void Http::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    if (responseHeader.statusCode() != 200) {
        QMessageBox::information(this, tr("QHttp"),
                                 tr("Download failed: %1")
                                 .arg(responseHeader.reasonPhrase()));
        httpRequestAborted = true;
        progressDialog->hide();
        http->abort();
        return;
    }
}

void Http::updateDataReadProgress(int bytesRead, int totalBytes)
{
    if (httpRequestAborted)
        return;

    progressDialog->setTotalSteps(totalBytes);
    progressDialog->setProgress(bytesRead);
}

void Http::enableDownloadButton()
{
    downloadButton->setEnabled(!urlLineEdit->text().isEmpty());
}
