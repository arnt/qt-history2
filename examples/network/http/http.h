#ifndef HTTP_H
#define HTTP_H

#include <QDialog>

class QFile;
class QHttp;
class QHttpResponseHeader;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;

class Http : public QDialog
{
    Q_OBJECT
public:
    Http(QWidget *parent = 0);

private slots:
    void download();
    void cancelDownload();
    void httpRequestFinished(int id, bool error);
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
