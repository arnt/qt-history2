#include <qstring.h>
#include <qstringlist.h>
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
};
