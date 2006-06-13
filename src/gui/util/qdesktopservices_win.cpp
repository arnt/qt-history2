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

#include "qdesktopservices.h"
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

/*!
   CreateInternetShortcut()

   pszShortcut - Path and file name of the Internet shortcut file. This
   must have the URL extension for the shortcut to be used correctly.

   pszURL - URL to be stored in the Internet shortcut file.
**/

HRESULT CreateInternetShortcut(LPTSTR pszShortcut, LPTSTR pszURL)
{
    IUniformResourceLocator *purl;
    HRESULT                 hr;

    hr = CoInitialize(NULL);

    if(SUCCEEDED(hr)) {
        //Get a pointer to the IShellLink interface.
        hr = CoCreateInstance(CLSID_InternetShortcut, NULL,
                              CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (LPVOID*)&purl);
    if(SUCCEEDED(hr)) {
        IPersistFile* ppf;

      hr = purl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

      if(SUCCEEDED(hr))
         {
         hr = purl->SetURL(pszURL, 0);

         if(SUCCEEDED(hr))
            {
            WCHAR wszShortcut[MAX_PATH];

#ifdef UNICODE
            lstrcpyn(wszShortcut, pszShortcut, MAX_PATH);
#else
            MultiByteToWideChar( CP_ACP,
                                 0,
                                 pszShortcut,
                                 -1,
                                 wszShortcut,
                                 MAX_PATH);
#endif

            hr = ppf->Save(wszShortcut, FALSE);
            }
         ppf->Release();
         }
       purl->Release();
       }

   CoUninitialize();
   }

   return hr;
}

bool QDesktopServices::launchWebBrowser(const QUrl &url)
{
    if (url.scheme() == "mailto" && url.toEncoded().length() >= 2083){
        QTemporaryFile temp(QDir::tempPath() + "/" + "qt_XXXXXX.url");
        temp.setAutoRemove(false);
        if (!temp.open())
            return false;
        temp.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
        QString encUrl = QString(url.toEncoded());
        QString fileName = temp.fileName();
        CreateInternetShortcut((TCHAR *)(fileName.utf16()), (TCHAR *)(encUrl.utf16()));
        QT_WA({
            ShellExecute(0, 0,(TCHAR *)(fileName.utf16()), 0, 0, SW_SHOWNORMAL);
        } , {
            ShellExecuteA(0, 0, fileName.toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
        });
        // http://support.microsoft.com/kb/q263909/
        // The temporary file can be safely deleted after calling ShellExecute
        QFile::remove(fileName);
        return true;
    }

    return openDocument(url);
}

bool QDesktopServices::openDocument(const QUrl &file)
{
    if (!file.isValid())
        return false;

    QT_WA({
                ShellExecute(0, 0, (TCHAR*)QString(file.toEncoded()).utf16(), 0, 0, SW_SHOWNORMAL);
            } , {
                ShellExecuteA(0, 0, QString(file.toEncoded()).toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
            });

    return true;
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

