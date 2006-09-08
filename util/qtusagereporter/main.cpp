/* qtusagereporter reports back qt usage to a secure trolltech server
   usage: qtusagereporter [nameofqtapp]

todo:
- test with unicode names(chinese user names etc.)
- make binaries for selected platforms
- coordinate and test with finns tool

possible problems:
- no network*
- no license key*
- non readable license key*
- qt3 license key (no LICENSEKEYEXT)*
- wrong license key
- expired license key
- firewall blocking
- using other customers license keys
*/
#include "qtsslsocket.h"
#include <QtNetwork>
#include <QtCore>
#include <iostream>

QString qgetusername() {
    QString qtuser = qgetenv("QTUSER");
    if (!qtuser.isEmpty())
        return qtuser;

#if defined(Q_OS_WIN)
    return qgetenv("USERNAME");
#else
    return qgetenv("USER");
#endif
}

void exit(QString errorText, int exitCode) {
    std::cout << "Error: " << qPrintable(errorText) << std::endl;
    exit(exitCode);
}

/* shamelessly copied from configureapp.cpp */
QString firstLicensePath()
{
    QStringList allPaths;
    allPaths << "./.qt-license"
             << QString::fromLocal8Bit(getenv("HOME")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("USERPROFILE")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("HOMEDRIVE"))
        + QString::fromLocal8Bit(getenv("HOMEPATH")) + "/.qt-license";
    for (int i = 0; i< allPaths.count(); ++i)
        if (QFile::exists(allPaths.at(i)))
            return allPaths.at(i);
    return QString();
}

QString qgetlicensekey() {
    if (firstLicensePath().isEmpty())
        exit("No .qt-license file found.", 1);

    QFile licenseFile(firstLicensePath());
    QMap<QString,QString> licenseInfo;
    if (licenseFile.open(QFile::ReadOnly)) {
        QString buffer = licenseFile.readLine(1024);
        while(!buffer.isEmpty()) {
            if (buffer[0] != '#') {
                QStringList components = buffer.split( '=');
                if (components.size() >= 2) {
                    QStringList::Iterator it = components.begin();
                    QString key = (*it++).trimmed().replace( "\"", QString()).toUpper();
                    QString value = (*it++).trimmed().replace( "\"", QString());
                    licenseInfo[key] = value;
                }
            }
            // read next line
            buffer = licenseFile.readLine(1024);
        }
        licenseFile.close();
    } else {
        exit(QString("Unable to read %1").arg(firstLicensePath()), 1);
    }
    if (licenseInfo["LICENSEKEYEXT"].isEmpty())
        exit(QString("Old or invalid .qt-license file. No LICENSEKEYEXT found in %1")
             .arg(firstLicensePath()), 1);
    return licenseInfo["LICENSEKEYEXT"];
}

/*!
  Returns true if this is the first time a Qt application has been run today.
*/
bool firstRun() {
    QSettings settings(QSettings::UserScope, "Trolltech");

    if (settings.value("Qt/lastused").toDate() >= QDate::currentDate())
        return false;

    settings.setValue("Qt/lastused", QDate::currentDate());
    return true;
}

/*!
  Returns the path of the CA file used for verifying that the key is valid
*/
QString caFile() {
    QFile fileCA(QCoreApplication::applicationDirPath() + "/" + "7651b327.0");
    QFile tempFileCA(QDir::tempPath() + "/" + "7651b327.0");

    if (fileCA.exists())
        return QFileInfo(fileCA).absoluteFilePath();

    if (tempFileCA.exists())
        return QFileInfo(tempFileCA).absoluteFilePath();

    if (!fileCA.open(QIODevice::WriteOnly))
        tempFileCA.open(QIODevice::WriteOnly);

    if (!fileCA.isOpen() && !tempFileCA.isOpen())
        exit("Failed to open CA file for writing", 1);

    QFile embeddedCA(":/certs/7651b327.0");
    if (embeddedCA.open(QIODevice::ReadOnly)) {
        if (fileCA.isOpen())
            fileCA.write(embeddedCA.readAll());
        else
            tempFileCA.write(embeddedCA.readAll());
        embeddedCA.close();
    } else {
        fileCA.isOpen() ? fileCA.close() : tempFileCA.close();
        exit("Failed to open embedded CA certificate", 1);
    }

    if (fileCA.isOpen()) {
        fileCA.close();
        return QFileInfo(fileCA).absoluteFilePath();
    } else {
        tempFileCA.close();
        return QFileInfo(tempFileCA).absoluteFilePath();
    }
}

class MyHttp : public QHttp {
    Q_OBJECT
public:
    MyHttp(const QString &hostName, const QString &caFile,
           quint16 port = 80, QObject *parent = 0)
        : QHttp(hostName, port, parent), postId(-1), serverName(hostName)
        {
            sslSocket.setPathToCACertFile(caFile);
            setSocket(&sslSocket);
            connect(&sslSocket,
                    SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult,
                                                      bool, const QString&)),
                    SLOT(verifySecureConnection(QtSslSocket::VerifyResult,
                                                bool, const QString&)));
            connect(this, SIGNAL(requestFinished(int, bool)), SLOT(finished(int, bool)));
            connect(this, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
                    SLOT(readResponseHeader(const QHttpResponseHeader &)));
        };

public Q_SLOTS:
    void finished(int id, bool error)
        {
            if (error)
                exit(errorString(), 1);
            if (id == postId) {
                std::cout << qPrintable(serverName) << " replied: "
                          << buffer.buffer().constData() << std::endl;
                QCoreApplication::quit();
            }
        }

    void readResponseHeader(const QHttpResponseHeader &responseHeader)
        {
            if (responseHeader.statusCode() != 200)
                exit(responseHeader.reasonPhrase(), 1);
        }

    void verifySecureConnection(QtSslSocket::VerifyResult result, bool hostNameMatched,
                                const QString &description)
        {
            if (hostNameMatched && result == QtSslSocket::VerifyOk) {
                std::cout << "Secure connection aquired: " << qPrintable(description) << std::endl;
            } else {
                close();
                exit(QString("Failed to open a secure connection: %1").arg(description), 1);
            }
        }

public:
    void reportUserData(const QString &username, const QString &licensekey,
                        const QString &application, const QString &date, const QString &platform)
        {
            QStringList postList;
            postList << "username=" + username;
            postList << "licensekey=" + licensekey;
            postList << "application=" + application;
            postList << "date=" + date;
            postList << "platform=" + platform;
            QString postString = postList.join("&").replace(' ', '+');
            postId = post("/geitekillingen", postString.toLatin1(), &buffer);
        }

public:
    int postId;
    QString serverName;
    QtSslSocket sslSocket;
    QBuffer buffer;
};

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    if (argc != 2) {
        std::cout << "Usage: ./qtusagereporter [name of Qt application]\n";
        return 0;
    }

    if (!firstRun())
        return 0;

    QString username(qgetusername());
    QString application(argv[1]);
    QString licensekey(qgetlicensekey());
    QString date(QDate::currentDate().toString("dd.MM.yyyy"));
    QString platform("n/a");
#if defined(Q_OS_UNIX)
    platform = "Unix";
#elif defined(Q_OS_WIN)
    platform = "Windows";
#elif defined(Q_OS_MAC)
    platform = "Mac OS";
#endif

    QString trolltechServer = "www.trolltech.com";
    std::cout << qPrintable(QString("Reporting Qt usage to %1\n").arg(trolltechServer));
    MyHttp http(trolltechServer, caFile(), 443);
    http.reportUserData(username, licensekey, application, date, platform);

    app.exec();
    return 0;
}

#include "main.moc"
