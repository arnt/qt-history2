#ifndef FTPWINDOW_H
#define FTPWINDOW_H

#include <QDialog>
#include <QHash>

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
    void connectToFtpServer();
    void downloadFile();
    void cancelDownload();

    void ftpCommandFinished(int commandId, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void processItem(QListWidgetItem *item);
    void cdToParent();
    void updateDataTransferProgress(qint64 readBytes,
                                    qint64 totalBytes);
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

    QHash<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp;
    QFile *file;
};

#endif
