/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include "qdesktop.h"
#include "qdebug.h"

#ifndef QT_NO_DESKTOP

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qdir.h>
#include <private/qcore_mac_p.h> // Remove if released as a solution

/*
    Extends HFSUniStr255 with a QString conversion operator.
*/
class QHFSUniStr255 : public HFSUniStr255
{
public:
    operator QString()
    {
        QChar *charPointer =  reinterpret_cast<QChar*>(unicode);
        return QString(charPointer, length);
    }
};

/*
    Translates a QDesktop::Location into the mac equivalent.
*/
/*
OSType translateLocation(Location type)
{
    switch (type) {
        case QDesktop::Desktop:
            return kDesktopFolderType; break;

        case QDesktop::Documents:
            return kDocumentsFolderType; break;

        case QDesktop::Fonts:
            // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
            // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
            return kFontsFolderType; break;

        case QDesktop::Applications:
            return kApplicationsFolderType; break;

        case QDesktop::Music:
            return kMusicDocumentsFolderType; break;

        case QDesktop::Movies:
            return kMovieDocumentsFolderType; break;

        case QDesktop::Pictures:
            return kPictureDocumentsFolderType; break;


        case QDesktop::Temp:
            return kTemporaryFolderType; break;

        default:
            return kDesktopFolderType; break;
    }
}
*/
/*
    Returns wether the given fsRef is something valid.
*/
static Boolean FSRefIsValid( const FSRef &fsRef )
{
  return ( FSGetCatalogInfo( &fsRef, kFSCatInfoNone, NULL, NULL, NULL, NULL ) == noErr );
}

/*
    Constructs a full unicode path from a FSRef (I can't find a system function
    that does this.. )
*/
static QString getFullPath(FSRef ref)
{
    QString path;
    FSRef parent;
    do {
        QHFSUniStr255 name;
        OSErr err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, &name, NULL, &parent);
        if (err)
            return QString();

        if (FSRefIsValid(parent))
            path.prepend(static_cast<QString>(name) + "/");
        ref = parent;
    } while (FSRefIsValid(ref));

    path.prepend("/");
    return path;
}

static bool lsOpen(const QUrl &url)
{
    if (!url.isValid())
        return false;

    QCFType<CFURLRef> cfUrl = CFURLCreateWithString(0, QCFString(url.toEncoded()), 0);
    if (cfUrl == 0)
        return false;

    const OSStatus err = LSOpenCFURLRef(cfUrl, 0);
    return (err == noErr);
}

bool QDesktop::launchMailComposer(const QString &to, const QString &subject, const QString &body)
{
    const QString url = QString("mailto:%1?subject='%2'&body='%3'").arg(to).arg(subject).arg(body);
    return lsOpen(url);
}

bool QDesktop::launchMailComposer(const QStringList &to, const QString &subject, const QString &body)
{
    return launchMailComposer(to.join(","), subject, body);
}

bool QDesktop::launchWebBrowser(const QUrl &url)
{
    return lsOpen(url);
}

bool QDesktop::open(const QUrl &file)
{
    if (!file.isValid())
        return false;

   // LSOpen does not work in this case, use QProcess open instead.
   return QProcess::startDetached("open", QStringList() << file.toLocalFile());
}
/*
QString QDesktop::storageLocation(const Location type)
{
     if (QDesktop::Home == type)
        return QDir::homePath();

     // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/folder_manager_ref/chapter_1.4_section_7.htm
     FSRef ref;
     OSErr err = FSFindFolder(kOnAppropriateDisk, translateLocation(type), false, &ref);
     if (err)
        return QString();

     return getFullPath(ref);
}

QString QDesktop::displayName(const Location type)
{
    if(QDesktop::Home == type)
        return QObject::tr("Home");

    FSRef ref;
    OSErr err = FSFindFolder(kOnAppropriateDisk, translateLocation(type), false, &ref);
    if (err)
        return QString();

    QCFString displayName;
    err = LSCopyDisplayNameForRef(&ref, &displayName);
    if (err)
        return QString();

    return static_cast<QString>(displayName);
}*/


#endif // QT_NO_DESKTOP

