#ifndef SHELL_H
#define SHELL_H

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#if defined(Q_OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#endif

class WinShell
{
public:
    WinShell();
    ~WinShell();

private:
    bool createDir( QString fullPath );
#if defined(Q_OS_WIN32)
    QString OLESTR2QString( LPOLESTR str );
#endif
public:
    QString localProgramsFolderName;
    QString commonProgramsFolderName;
    QString windowsFolderName;
    QString selectFolder( QString folderName, bool common );

    QString createFolder( QString folderName, bool common );

#if defined(Q_OS_WIN32)
    HRESULT createShortcut( QString folderName, bool common, QString shortcutName, QString target, QString description = QString(), QString arguments = QString(), QString wrkDir = QString() );
    void createInternetShortcut( QString folderName, bool common, QString shortcutName, QString url );
#endif

    static QPixmap* getOpenFolderImage();
    static QPixmap* getClosedFolderImage();
    static QPixmap* getFileImage();
    static QPixmap* getInfoImage();
#if defined(Q_OS_WIN32)
    static ULARGE_INTEGER dirFreeSpace( QString dirPath );
#elif defined(Q_OS_MAC)
    static long dirFreeSpace( QString dirPath );
#endif
};

#endif
