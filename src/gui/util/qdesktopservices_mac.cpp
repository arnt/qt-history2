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

#ifndef QT_NO_DESKTOPSERVICES

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
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
    Translates a QDesktopServices::Location into the mac equivalent.
*/
/*
OSType translateLocation(Location type)
{
    switch (type) {
    case QDesktopServices::Desktop:
        return kDesktopFolderType; break;

    case QDesktopServices::Documents:
        return kDocumentsFolderType; break;

    case QDesktopServices::Fonts:
        // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
        // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
        return kFontsFolderType; break;

    case QDesktopServices::Applications:
        return kApplicationsFolderType; break;

    case QDesktopServices::Music:
        return kMusicDocumentsFolderType; break;

    case QDesktopServices::Movies:
        return kMovieDocumentsFolderType; break;

    case QDesktopServices::Pictures:
        return kPictureDocumentsFolderType; break;


    case QDesktopServices::Temp:
        return kTemporaryFolderType; break;

    default:
        return kDesktopFolderType; break;
    }
}
*/
static bool lsOpen(const QUrl &url)
{
    if (!url.isValid())
        return false;

    QCFType<CFURLRef> cfUrl = CFURLCreateWithString(0, QCFString(QString::fromLatin1(url.toEncoded())), 0);
    if (cfUrl == 0)
        return false;

    const OSStatus err = LSOpenCFURLRef(cfUrl, 0);
    return (err == noErr);
}

static bool launchWebBrowser(const QUrl &url)
{
    return lsOpen(url);
}

static bool openDocument(const QUrl &file)
{
    if (!file.isValid())
        return false;

   // LSOpen does not work in this case, use QProcess open instead.
   return QProcess::startDetached(QLatin1String("open"), QStringList() << file.toLocalFile());
}

#if 0
/*
    Returns wether the given fsRef is something valid.
*/
static Boolean FSRefIsValid(const FSRef &fsRef)
{
  return (FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, NULL, NULL) == noErr);
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

QString QDesktopServices::storageLocation(const Location type)
{
     if (QDesktopServices::Home == type)
        return QDir::homePath();

     // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/folder_manager_ref/chapter_1.4_section_7.htm
     FSRef ref;
     OSErr err = FSFindFolder(kOnAppropriateDisk, translateLocation(type), false, &ref);
     if (err)
        return QString();

     return getFullPath(ref);
}

QString QDesktopServices::displayName(const Location type)
{
    if (QDesktopServices::Home == type)
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
}
#endif


#endif // QT_NO_DESKTOPSERVICES

