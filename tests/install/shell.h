#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <windows.h>
#include <shlobj.h>

class WinShell
{
public:
    WinShell();
    ~WinShell();

private:
    IShellFolder* desktopFolder;
    IShellFolder* localProgramsFolder;
    IShellFolder* commonProgramsFolder;

    HRESULT EnumFolder( IShellFolder* folder, QStringList& entryList );
    bool createDir( QString fullPath );
    QByteArray QString2OLESTR( QString );
public:
    QString localProgramsFolderName;
    QString commonProgramsFolderName;
    QString selectFolder( QString folderName, bool common );

    QString createFolder( QString folderName, bool common );
    HRESULT createShortcut( QString folderName, bool common, QString shortcutName, QString target, QString description = QString::null, QString arguments = QString::null );

    static QPixmap* getOpenFolderImage();
    static QPixmap* getClosedFolderImage();
    static QPixmap* getFileImage();
    static QPixmap* getInfoImage();
};
