#include <qstring.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qobject.h>

class QArchive : public QObject
{
    Q_OBJECT;
public:
    QArchive( const QString& archivePath = QString:: null );
    ~QArchive();

    void setPath( const QString& archivePath );
    void setVerbosity( int verbosity );

    bool open( int mode );
    void close();
    bool isOpen() { return arcFile.isOpen(); }

    bool writeFile( const QString& fileName, const QString& localPath = QString::null );
    bool writeFileList( const QStringList fileList );
    bool writeDir( const QString& dirName, bool includeLastComponent = false, QString localPath = QString::null );
    bool writeDirList( const QStringList dirList, bool includeLastComponent = true );

    bool readArchive( QString outpath );
private:
    QFile arcFile;

    int bufferSize;
    int verbosityMode;
    int opened;
    bool setDirectory( const QString& dirName );
signals:
    void operationFeedback( const QString& );

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
