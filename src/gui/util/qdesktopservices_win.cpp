/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsettings.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qprocess.h>
#include <qtemporaryfile.h>

#include <windows.h>
#include <shlobj.h>
#include <intshcut.h>

#ifndef QT_NO_DESKTOPSERVICES

//#undef UNICODE

static bool openDocument(const QUrl &file)
{
    if (!file.isValid())
        return false;

    QT_WA({
                ShellExecute(0, 0, (TCHAR *)file.toString().utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(0, 0, file.toString().toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);
            });

    return true;
}

static bool launchWebBrowser(const QUrl &url)
{
    if (url.scheme() == QLatin1String("mailto")) {
        //Retrieve the commandline for the default mail cleint
        //the key used below is the command line for the mailto: shell command
        long  bufferSize = 2*MAX_PATH;    
        long  returnValue =  -1;
        QString command;
        QT_WA ({
            wchar_t subKey[] = L"mailto\\shell\\open\\command";    
            wchar_t keyValue[2*MAX_PATH];    
            returnValue = RegQueryValue(HKEY_CLASSES_ROOT, subKey, keyValue, &bufferSize);
            if (!returnValue)
                command = QString::fromRawData((QChar*)keyValue, bufferSize);
        }, {
            char subKey[] = "mailto\\shell\\open\\command";    
            char keyValue[2*MAX_PATH];    
            returnValue = RegQueryValueA(HKEY_CLASSES_ROOT, subKey, keyValue, &bufferSize);
            if (!returnValue)
                command = QString::fromLocal8Bit(keyValue);
        });
        if(returnValue)
            return false;
        command = command.trimmed();
        //Make sure the path for the process is in quotes
        int index = -1 ;
        if (command[0]!= QLatin1Char('\"')) {
            index = command.indexOf(QLatin1String(".exe "), 0, Qt::CaseInsensitive);
            command.insert(index+4, QLatin1Char('\"'));
            command.insert(0, QLatin1Char('\"'));
        }
        //pass the url as the parameter
        index =  command.lastIndexOf(QLatin1String("%1"));
        if (index != -1){
            command.replace(index, 2, url.toString());
        }
        //start the process
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        QT_WA ({
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            returnValue = CreateProcess(NULL, (TCHAR*)command.utf16(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        }, {
            STARTUPINFOA si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);

            returnValue = CreateProcessA(NULL, command.toLocal8Bit().data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        });

        if (!returnValue)
            return false;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    return openDocument(url);
}

/*
QString QDesktopServices::storageLocation(const Location type)
{
    QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
    settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
    switch (type) {
    case Desktop:
        return settings.value("Desktop").toString();
        break;

    case Documents:
        return settings.value("Personal").toString();
        break;

    case Fonts:
        return settings.value("Fonts").toString();
        break;

    case Applications:
        return settings.value("Programs").toString();
        break;

    case Music:
        return settings.value("My Music").toString();
        break;

    case Movies:
        return settings.value("My Video").toString();
        break;

    case Pictures:
        return settings.value("My Pictures").toString();
        break;

    case QDesktopServices::Home:
        return QDir::homePath(); break;

    case QDesktopServices::Temp:
        return QDir::tempPath(); break;

    default:
        break;
    }

    return QString();
}

QString QDesktopServices::displayName(const Location type)
{
    Q_UNUSED(type);
    return QString();
}
*/
#endif // QT_NO_DESKTOPSERVICES

