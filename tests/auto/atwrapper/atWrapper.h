#ifndef ATWRAPPER_H
#define ATWRAPPER_H

#include <QHash>
#include <QString>
#include <QUrlInfo>

class atWrapper : public QObject
{
    Q_OBJECT

    public:
        atWrapper();
        bool runAutoTests();

    private:
        bool executeTests();
        bool initTests(bool *haveBaseline);
        bool compare();
        void createBaseline();
        bool loadConfig( QString );
        void compareDirs( QString, QString );
        bool diff( QString, QString, QString );
        void downloadBaseline();
        void uploadFailed( QString, QString, QByteArray );
        bool ftpMkDir( QString );
        void ftpRmDir( QString );
        bool setupFTP();
        void uploadDiff( QString, QString, QString );

        QHash<QString, QString> enginesToTest;
        QString framework;
        QString suite;
        QString output;
        QString ftpUser;
        QString ftpPass;
        QString ftpHost;
        QString ftpBaseDir;
        QList<QString> rmDirList;
        QList<QString> mgetDirList;
        QString configPath;

    private slots:
        void ftpRmDirAddToList( const QUrlInfo &urlInfo );
        void ftpRmDirDone( bool );
        void ftpMgetAddToList( const QUrlInfo &urlInfo );
        void ftpMgetDone( bool );
};

#endif
