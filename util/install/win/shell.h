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
    bool createDir( QString fullPath );
    QString OLESTR2QString( LPOLESTR str );
public:
    QString localProgramsFolderName;
    QString commonProgramsFolderName;
    QString windowsFolderName;
    QString selectFolder( QString folderName, bool common );

    QString createFolder( QString folderName, bool common );
    HRESULT createShortcut( QString folderName, bool common, QString shortcutName, QString target, QString description = QString::null, QString arguments = QString::null, QString wrkDir = QString::null );

    static QPixmap* getOpenFolderImage();
    static QPixmap* getClosedFolderImage();
    static QPixmap* getFileImage();
    static QPixmap* getInfoImage();
    static ULARGE_INTEGER dirFreeSpace( QString dirPath );
};
