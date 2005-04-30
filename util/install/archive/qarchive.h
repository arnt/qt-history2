#ifndef QARCHIVE_H
#define QARCHIVE_H

#include <qstring.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qobject.h>

class QArchiveHeader
{
public:
    QArchiveHeader( uint feat, const QString& desc=QString(), uchar mayorVer = 1, uchar minorVer = 0 )
	: _features(feat), _description(desc), _mayorVersion(mayorVer), _minorVersion(minorVer)
    {}
    
    uint features() const
    { return _features; }

    QString description() const
    { return _description; }

    uchar mayorVersion() const
    { return _mayorVersion; }

    uchar minorVersion() const
    { return _minorVersion; }

    void addExtraData( const QString& key, const QString& value )
    { extraData.insert( key, value ); }

    QString findExtraData( const QString& key ) const
    { return extraData[key]; }

private:
    QArchiveHeader()
    {}

    uint _features;
    QString _description;
    uchar _mayorVersion;
    uchar _minorVersion;
    QMap<QString,QString> extraData;

    friend class QArchive;
};

class QArchive : public QObject
{
    Q_OBJECT
public:
    QArchive( const QString& archivePath = QString() );
    ~QArchive();

    void setPath( const QString& archivePath );
    void setVerbosity( int verbosity );

    bool symbolicLinks() const { return doSyms; }
    void setSymbolicLinks(bool b) { doSyms = b; }

    bool open( int mode );
    void close();
    bool isOpen() { return arcFile.isOpen(); }

    bool writeHeader( const QArchiveHeader header );
    bool writeFile( const QString& fileName, const QString& localPath = QString() );
    bool writeFileList( const QStringList fileList );
    bool writeDir( const QString& dirName, bool includeLastComponent = false, 
		   const QString& localPath = QString() );
    bool writeDirList( const QStringList dirList, bool includeLastComponent = true );

    QArchiveHeader* readArchiveHeader();
    QArchiveHeader* readArchiveHeader( QDataStream *inStream );

    bool readArchive( const QString &outpath, const QString &key = QString() );
    bool readArchive( QDataStream *inStream, const QString &outpath, const QString &key = QString() );
private:
    QFile arcFile;

    int bufferSize;
    int verbosityMode;
    uint doSyms : 1;
    bool setDirectory( const QString& dirName );
signals:
    void operationFeedback( const QString& );
    void operationFeedback( int );

public:
    enum {
	NoFeedback = 0x00,
	OnlyNames = 0x01,
	Verbose = 0x02,
	Source = 0x10,
	Destination = 0x20,
	Progress = 0x40
    };
};

#endif
