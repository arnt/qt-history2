#include <qstring.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qobject.h>

class QArchive : public QObject
{
    Q_OBJECT
public:
    QArchive( const QString& archivePath = QString::null );
    ~QArchive();

    void setPath( const QString& archivePath );
    void setVerbosity( int verbosity );

    bool symbolicLinks() const { return doSyms; }
    void setSymbolicLinks(bool b) { doSyms = b; }

    bool open( int mode );
    void close();
    bool isOpen() { return arcFile.isOpen(); }

    bool writeFeatures( uint features );
    bool writeFile( const QString& fileName, const QString& localPath = QString::null );
    bool writeFileList( const QStringList fileList );
    bool writeDir( const QString& dirName, bool includeLastComponent = false, 
		   const QString& localPath = QString::null );
    bool writeDirList( const QStringList dirList, bool includeLastComponent = true );

    bool readArchive( const QString &outpath, const QString &key = QString::null );
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
