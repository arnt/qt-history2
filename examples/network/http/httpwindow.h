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
