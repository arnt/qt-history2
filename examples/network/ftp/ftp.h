#ifndef FTP_H
#define FTP_H

#include <QDialog>
#include <QMap>

class QFile;
class QFtp;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressDialog;
class QPushButton;
class QUrlInfo;

class Ftp : public QDialog
{
    Q_OBJECT
public:
    Ftp(QWidget *parent = 0);

private slots:
    void connectToFtpServer();
    void downloadFile();
    void cancelDownload();

    void ftpCommandFinished(int id, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void processItem(QListWidgetItem *item);
    void cdToParent();
    void updateDataTransferProgress(Q_LLONG readBytes, Q_LLONG totalBytes);
    void enableConnectButton();
    void enableDownloadButton();

private:
    QLabel *ftpServerLabel;
    QLineEdit *ftpServerLineEdit;
    QLabel *statusLabel;
    QListWidget *fileList;
    QPushButton *quitButton;
    QPushButton *connectButton;
    QPushButton *downloadButton;
    QPushButton *cdToParentButton;
    QProgressDialog *progressDialog;

    QMap<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp;
    QFile *file;
};

#endif

