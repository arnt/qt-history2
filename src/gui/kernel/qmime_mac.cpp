/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmime.h"


//#define USE_INTERNET_CONFIG

#ifndef USE_INTERNET_CONFIG
# include "qfile.h"
# include "qfileinfo.h"
# include "qtextstream.h"
# include "qdir.h"
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/fcntl.h>
#endif

#include "qpixmap.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qdatetime.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qurl.h"
#include "qmap.h"
#include <private/qt_mac_p.h>

typedef QList<QMacMime*> MimeList;
Q_GLOBAL_STATIC(MimeList, globalMimeList)

static void cleanup_mimes()
{
    MimeList *mimes = globalMimeList();
    while (!mimes->isEmpty())
        delete mimes->takeFirst();
}

/*****************************************************************************
  QDnD debug facilities
 *****************************************************************************/
//#define DEBUG_MIME_MAPS

//functions
extern QString qt_mac_from_pascal_string(const Str255);  //qglobal.cpp
extern void qt_mac_from_pascal_string(QString, Str255, TextEncoding encoding=0, int len=-1);  //qglobal.cpp
OSErr qt_mac_create_fsspec(const QString &path, FSSpec *spec); //qglobal_mac.cpp

ScrapFlavorType qt_mac_mime_type = 'CUTE';
CFStringRef qt_mac_mime_typeUTI = CFSTR("com.pasteboard.trolltech.marker");

/*!
  \class QMacMime
  \brief The QMacMime class maps open-standard MIME to Mac flavors.
  \ingroup io
  \ingroup draganddrop
  \ingroup misc

  Qt's drag and drop support and clipboard facilities use the MIME
  standard. On X11, this maps trivially to the Xdnd protocol, but on
  Mac although some applications use MIME types to describe clipboard
  formats, others use arbitrary non-standardized naming conventions,
  or unnamed built-in Mac formats.

  By instantiating subclasses of QMacMime that provide conversions
  between Mac flavors and MIME formats, you can convert proprietary
  clipboard formats to MIME formats.

  Qt has predefined support for the following Mac flavors:
  \list
    \i kScrapFlavorTypeUnicode - converted to "text/plain;charset=ISO-10646-UCS-2"
    \i kScrapFlavorTypeText - converted to "text/plain;charset=system" or "text/plain"
    \i kScrapFlavorTypePicture - converted to "application/x-qt-image"
    \i typeFileURL - converted to "text/uri-list"
  \endlist

  You can check if a MIME type is convertible using canConvert() and
  can perform conversions with convertToMime() and convertFromMime().
*/

/*! \enum QMacMime::QMacMimeType
    \internal
*/

/*!
  Constructs a new conversion object of type \a t, adding it to the
  globally accessed list of available convertors.
*/
QMacMime::QMacMime(char t) : type(t)
{
    globalMimeList()->append(this);
}

/*!
  Destroys a conversion object, removing it from the global
  list of available convertors.
*/
QMacMime::~QMacMime()
{
    if(!QApplication::closingDown())
        globalMimeList()->removeAll(this);
}

class QMacMimeAnyMime : public QMacMime {
private:

public:
    QMacMimeAnyMime() : QMacMime(MIME_QT_CONVERTOR|MIME_ALL) {
    }
    ~QMacMimeAnyMime() {
    }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

QString QMacMimeAnyMime::flavorFor(const QString &mime)
{
    QString ret = "com.trolltech.anymime." + mime;
    return ret.replace("/", "--");
}

QString QMacMimeAnyMime::mimeFor(QString flav)
{
    const QString any_prefix = "com.trolltech.anymime.";
    if(flav.size() > any_prefix.length() && flav.startsWith(any_prefix))
        return flav.mid(any_prefix.length()).replace("--", "/");
    return QString();
}

bool QMacMimeAnyMime::canConvert(const QString &mime, QString flav)
{
    return mimeFor(flav) == mime;
}

QVariant QMacMimeAnyMime::convertToMime(const QString &mime, QList<QByteArray> data, QString)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: Cannot handle multiple member data");
    QVariant ret;
    if (mime == "text/plain") {
        ret = QString::fromUtf8(data.first());
    } else {
        ret = data.first();
    }
    return ret;
}

QList<QByteArray> QMacMimeAnyMime::convertFromMime(const QString &mime, QVariant data, QString)
{
    QList<QByteArray> ret;
    if (mime == "text/plain") {
        ret.append(data.toString().toUtf8());
    } else {
        ret.append(data.toByteArray());
    }
    return ret;
}

class QMacMimeText : public QMacMime {
public:
    QMacMimeText() : QMacMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacMimeText::convertorName()
{
    return "Text";
}

QString QMacMimeText::flavorFor(const QString &mime)
{
#if 1
    if(mime == QLatin1String("text/plain"))
        return QCFString("com.apple.traditional-mac-plain-text");
#endif

    if(mime == QLatin1String("text/plain"))
        return QCFString(kUTTypeUTF16PlainText);
    int i = mime.indexOf(QLatin1String("charset="));
    if(i >= 0) {
        QString cs(mime.mid(i+8));
        i = cs.indexOf(";");
        if(i>=0)
            cs = cs.left(i);
        if(cs == QLatin1String("system"))
            return QCFString(kUTTypeUTF8PlainText);
        else if(cs == QLatin1String("ISO-10646-UCS-2") ||
                cs == QLatin1String("utf16"))
            return QCFString(kUTTypeUTF16PlainText);
    }
    return 0;
}

QString QMacMimeText::mimeFor(QString flav)
{
    if (flav == QCFString(kUTTypeUTF16PlainText) || flav == QCFString(kUTTypeUTF8PlainText) ||
        flav == QCFString("com.apple.traditional-mac-plain-text"))
        return QLatin1String("text/plain");
    return QString();
}

bool QMacMimeText::canConvert(const QString &mime, QString flav)
{
    return flavorFor(mime) == flav;
}

QVariant QMacMimeText::convertToMime(const QString &mimetype, QList<QByteArray> data, QString flavor)
{
    if(data.count() > 1)
        qWarning("QMacMimeText: Cannot handle multiple member data");
    const QByteArray &firstData = data.first();
    // I can only handle two types (system and unicode) so deal with them that way
    QVariant ret;
    if(flavor == QCFString(kUTTypeUTF8PlainText) || flavor == QCFString("com.apple.traditional-mac-plain-text")) {
        QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                             reinterpret_cast<const UInt8 *>(firstData.constData()),
                                             firstData.size(), CFStringGetSystemEncoding(), false));
        ret = QString(str);
    } else if (flavor == QCFString(kUTTypeUTF16PlainText)) {
        ret = QString::fromUtf16(reinterpret_cast<const ushort *>(firstData.constData()),
                                 firstData.size() / sizeof(ushort));
    } else {
        qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
    }
    return ret;
}

QList<QByteArray> QMacMimeText::convertFromMime(const QString &, QVariant data, QString flavor)
{
    QList<QByteArray> ret;
    QString string = data.toString();
    if(flavor == QCFString(kUTTypeUTF8PlainText) || flavor == QCFString("com.apple.traditional-mac-plain-text"))
        ret.append(string.toLatin1());
    else if (flavor == QCFString(kUTTypeUTF16PlainText))
        ret.append(QByteArray((char*)string.utf16(), string.length()*2));
    return ret;
}


class QMacMimeImage : public QMacMime {
public:
    QMacMimeImage() : QMacMime(MIME_ALL) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacMimeImage::convertorName()
{
    return "Image";
}

QString QMacMimeImage::flavorFor(const QString &mime)
{
    if(mime.startsWith(QLatin1String("application/x-qt-image")))
        return QCFString(kUTTypePICT);
    return QString();
}

QString QMacMimeImage::mimeFor(QString flav)
{
    if(flav == QCFString(kUTTypePICT))
        return QString("application/x-qt-image");
    return QString();
}

bool QMacMimeImage::canConvert(const QString &mime, QString flav)
{
    if(flav == QCFString(kUTTypePICT) && mime == QLatin1String("application/x-qt-image"))
        return true;
    return false;
}

QVariant QMacMimeImage::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: Cannot handle multiple member data");
    QVariant ret;
    if(mime != QLatin1String("application/x-qt-image") || flav == QCFString(kUTTypePICT))
        return ret;
    QByteArray &a = data.first();
    PicHandle pic = (PicHandle)NewHandle(a.size());
    memcpy(*pic, a.data(), a.size());
    PictInfo pinfo;
    if(GetPictInfo(pic, &pinfo, 0, 0, 0, 0) == noErr) {
        QPixmap px(pinfo.sourceRect.right - pinfo.sourceRect.left,
                   pinfo.sourceRect.bottom - pinfo.sourceRect.top);
        {
            Rect r; SetRect(&r, 0, 0, px.width(), px.height());
            QMacSavedPortInfo pi(&px);
            DrawPicture(pic, &r);
        }
        ret = QVariant(px.toImage());
    }
    DisposeHandle((Handle)pic);
    return ret;
}

QList<QByteArray> QMacMimeImage::convertFromMime(const QString &mime, QVariant variant, QString flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("application/x-qt-image") || flav != QCFString(kUTTypePICT))
        return ret;
    QImage img = qvariant_cast<QImage>(variant);

    OpenCPicParams pic_params;
    pic_params.version = -2; // Version field is always -2
    SetRect(&pic_params.srcRect, 0, 0, img.width(), img.height());
    pic_params.hRes = pic_params.vRes = 0x00480000; // 72 dpi
    PicHandle pic = OpenCPicture(&pic_params);
    {
	GWorldPtr world;
	GetGWorld(&world, 0);
        ClipRect(&pic_params.srcRect);
        QPixmap px = QPixmap::fromImage(img);
	CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.macQDHandle()),
                 GetPortBitMapForCopyBits((GWorldPtr)world),
                 &pic_params.srcRect, &pic_params.srcRect, srcCopy, 0);
    }
    ClosePicture();

    int size = GetHandleSize((Handle)pic);
    HLock((Handle)pic);
    QByteArray ar = QByteArray::fromRawData(reinterpret_cast<char *>(*pic), size);
    HUnlock((Handle)pic);
    ret.append(ar);
    return ret;
}


class QMacMimeFileUri : public QMacMime {
public:
    QMacMimeFileUri() : QMacMime(MIME_DND) { }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

QString QMacMimeFileUri::convertorName()
{
    return "FileURL";
}

QString QMacMimeFileUri::flavorFor(const QString &mime)
{
    if(mime == QLatin1String("text/uri-list"))
        return QCFString(kUTTypeFileURL);
    return QString();
}

QString QMacMimeFileUri::mimeFor(QString flav)
{
    if(flav == QCFString(kUTTypeFileURL))
        return QString("text/uri-list");
    return QString();
}

bool QMacMimeFileUri::canConvert(const QString &mime, QString flav)
{
    if(mime == QLatin1String("text/uri-list"))
        return flav == QCFString(kUTTypeFileURL);
    return false;
}

QVariant QMacMimeFileUri::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)
{
    if(mime != QLatin1String("text/uri-list") || flav != QCFString(kUTTypeFileURL))
        return QVariant();
    QList<QVariant> ret;
    for(int i = 0; i < data.size(); ++i) {
        const QByteArray &datum = data.at(i);
        QString url = QString::fromUtf8(datum, datum.size());
        if(url.startsWith(QLatin1String("file://localhost/"))) //mac encodes a bit differently
            url.remove(7, 9);
        ret.append(QUrl(url));
    }
    return QVariant(ret);
}

QList<QByteArray> QMacMimeFileUri::convertFromMime(const QString &mime, QVariant data, QString flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("text/uri-list") || flav != QCFString(kUTTypeFileURL))
        return ret;
    QList<QVariant> urls = data.toList();
    for(int i = 0; i < urls.size(); ++i) {
        QUrl url = urls.at(i).toUrl();
        QString uri;
        if(url.scheme().isEmpty())
            uri = QUrl::fromLocalFile(url.toString()).toString();
        else
            uri = url.toString();
        if(uri.startsWith(QLatin1String("file:///")))
            uri.insert(7, "localhost"); //Mac likes localhost to be in it!
        ret.append(uri.toUtf8());
    }
    return ret;
}

#ifdef QT3_SUPPORT
class QMacMimeQt3AnyMime : public QMacMime {
private:
    int current_max;
    QFile library_file;
    QDateTime mime_registry_loaded;
    QMap<QString, int> mime_registry;
    int registerMimeType(const QString &mime);
    bool loadMimeRegistry();

public:
    QMacMimeQt3AnyMime() : QMacMime(MIME_QT3_CONVERTOR) {
        current_max = 'QT00';
    }
    ~QMacMimeQt3AnyMime() {
    }
    QString convertorName();

    QString flavorFor(const QString &mime);
    QString mimeFor(QString flav);
    bool canConvert(const QString &mime, QString flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
};

static bool qt_mac_openMimeRegistry(bool global, QIODevice::OpenMode mode, QFile &file)
{
    QString dir = "/Library/Qt";
    if(!global)
        dir.prepend(QDir::homePath());
    file.setFileName(dir + "/.mime_types");
    if(mode != QIODevice::ReadOnly) {
        if(!QFile::exists(dir)) {
            // Do it with a system call as I don't see much worth in
            // doing it with QDir since we have to chmod anyway.
            bool success = ::mkdir(dir.toLocal8Bit().constData(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
            if (success)
                success = ::chmod(dir.toLocal8Bit().constData(), S_IRUSR | S_IWUSR | S_IXUSR
                                      | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) == 0;
            if (!success)
                return false;
        }
        if (!file.exists()) {
            // Create the file and chmod it so that everyone can write to it.
            int fd = ::open(file.fileName().toLocal8Bit().constData(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            bool success = fd != -1;
            if (success)
                success = ::fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == 0;
            if (fd != -1)
                ::close(fd);
            if(!success)
                return false;
        }
    }
    return file.open(mode);
}
static void qt_mac_loadMimeRegistry(QFile &file, QMap<QString, int> &registry, int &max)
{
    file.reset();
    QTextStream stream(&file);
    while(!stream.atEnd()) {
	QString mime = stream.readLine();
	int mactype = stream.readLine().toInt();
	if(mactype > max)
	    max = mactype;
	registry.insert(mime, mactype);
    }
}

bool QMacMimeQt3AnyMime::loadMimeRegistry()
{
    if(!library_file.isOpen()) {
        if(!qt_mac_openMimeRegistry(true, QIODevice::ReadWrite, library_file)) {
            QFile global;
            if(qt_mac_openMimeRegistry(true, QIODevice::ReadOnly, global)) {
                ::qt_mac_loadMimeRegistry(global, mime_registry, current_max);
                global.close();
            }
            if(!qt_mac_openMimeRegistry(false, QIODevice::ReadWrite, library_file)) {
                qWarning("QMacMimeAnyQt3Mime: Failure to open mime resources %s -- %s", library_file.fileName().toLatin1().constData(),
                         library_file.errorString().toLatin1().constData());
                return false;
            }
        }
    }

    QFileInfo fi(library_file);
    if(!mime_registry_loaded.isNull() && mime_registry_loaded == fi.lastModified())
        return true;
    mime_registry_loaded = fi.lastModified();
    ::qt_mac_loadMimeRegistry(library_file, mime_registry, current_max);
    return true;
}

int QMacMimeQt3AnyMime::registerMimeType(const QString &mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("QMacMimeAnyQt3Mime: Internal error");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            if(!library_file.isOpen()) {
                if(!library_file.open(QIODevice::WriteOnly)) {
                    qWarning("QMacMimeAnyQt3Mime: Failure to open %s -- %s", library_file.fileName().toLatin1().constData(),
                             library_file.errorString().toLatin1().constData());
                    return false;
                }
            }
            int ret = ++current_max;
            mime_registry_loaded = QFileInfo(library_file).lastModified();
            QTextStream stream(&library_file);
            stream << mime << endl;
            stream << ret << endl;
            mime_registry.insert(mime, ret);
            library_file.flush(); //flush and set mtime
            return ret;
        }
    }
    return mime_registry[mime];
}

QString QMacMimeQt3AnyMime::convertorName()
{
    return "Qt3-Any-Mime";
}

QString QMacMimeQt3AnyMime::flavorFor(const QString &mime)
{
    const int os_flav = registerMimeType(mime);
    QCFType<CFArrayRef> ids = UTTypeCreateAllIdentifiersForTag(0, kUTTagClassOSType,
                                                               QCFString(UTCreateStringForOSType(os_flav)));
    if(ids) {
        const int type_count = CFArrayGetCount(ids);
        if(type_count) {
            if(type_count > 1)
                qDebug("Can't happen!");
            return QCFString((CFStringRef)CFArrayGetValueAtIndex(ids, 0));
        }
    }
    return QString();
}

QString QMacMimeQt3AnyMime::mimeFor(QString flav)
{
    loadMimeRegistry();
    const int os_flav = UTGetOSTypeFromString(UTTypeCopyPreferredTagWithClass(QCFString(flav), kUTTagClassOSType));
    for(QMap<QString, int>::const_iterator it = mime_registry.constBegin();
        it != mime_registry.constEnd(); ++it) {
        if(it.value() == os_flav)
            return it.key().toLatin1();
    }
    return QString();
}

bool QMacMimeQt3AnyMime::canConvert(const QString &mime, QString flav)
{
    loadMimeRegistry();
    const int os_flav = UTGetOSTypeFromString(UTTypeCopyPreferredTagWithClass(QCFString(flav), kUTTagClassOSType));
    if(mime_registry.contains(mime) && mime_registry[mime] == os_flav)
        return true;
    return false;
}

QVariant QMacMimeQt3AnyMime::convertToMime(const QString &, QList<QByteArray>, QString)
{
    qWarning("QMacMimeAnyQt3Mime: Cannot write anything!");
    return QVariant();
}

QList<QByteArray> QMacMimeQt3AnyMime::convertFromMime(const QString &mime, QVariant data, QString)
{
    QList<QByteArray> ret;
    if (mime == "text/plain") {
        ret.append(data.toString().toUtf8());
    } else {
        ret.append(data.toByteArray());
    }
    return ret;
}
#endif

/*!
  \internal

  This is an internal function.
*/
void QMacMime::initialize()
{
    if(globalMimeList()->isEmpty()) {
        qAddPostRoutine(cleanup_mimes);

        //standard types that we wrap
        new QMacMimeImage;
        new QMacMimeText;
        new QMacMimeFileUri;

        //make sure our "non-standard" types are always last! --Sam
        new QMacMimeAnyMime;
#ifdef QT3_SUPPORT
        new QMacMimeQt3AnyMime;
#endif
    }
}

/*!
  Returns the most-recently created QMacMime of type \a t that can convert
  between the \a mime and \a flav formats.  Returns 0 if no such convertor
  exists.
*/
QMacMime*
QMacMime::convertor(uchar t, const QString &mime, QString flav)
{
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
        qDebug("QMacMime::convertor: seeing if %s (%d) can convert %s to %d[%c%c%c%c] [%d]",
               (*it)->convertorName().toLatin1().constData(),
               (*it)->type & t, mime.toLatin1().constData(),
               flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
               (*it)->canConvert(mime,flav));
        for(int i = 0; i < (*it)->countFlavors(); ++i) {
            int f = (*it)->flavor(i);
            qDebug("  %d) %d[%c%c%c%c] [%s]", i, f,
                   (f >> 24) & 0xFF, (f >> 16) & 0xFF, (f >> 8) & 0xFF, (f) & 0xFF,
                   (*it)->convertorName().toLatin1().constData());
        }
#endif
        if(((*it)->type & t) && (*it)->canConvert(mime, flav))
            return (*it);
    }
    return 0;
}
/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
QString QMacMime::flavorToMime(uchar t, QString flav)
{
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
#ifdef DEBUG_MIME_MAPS
        qDebug("QMacMIme::flavorToMime: attempting %s (%d) for flavor %d[%c%c%c%c] [%s]",
               (*it)->convertorName().toLatin1().constData(),
               (*it)->type & t, flav, (flav >> 24) & 0xFF, (flav >> 16) & 0xFF, (flav >> 8) & 0xFF, (flav) & 0xFF,
               (*it)->mimeFor(flav).toLatin1().constData());

#endif
        if((*it)->type & t) {
            QString mimeType = (*it)->mimeFor(flav);
            if(!mimeType.isNull())
                return mimeType;
        }
    }
    return QString();
}

/*!
  Returns a list of all currently defined QMacMime objects of type \a t.
*/
QList<QMacMime*> QMacMime::all(uchar t)
{
    MimeList ret;
    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
        if((*it)->type & t)
            ret.append((*it));
    }
    return ret;
}

/*!
  \fn QString QMacMime::convertorName()

  Returns a name for the convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QMacMime::canConvert(const QString &mime, QString flav)

  Returns true if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns false.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacMime::mimeFor(QString flav)

  Returns the MIME UTI used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacMime::flavorFor(const QString &mime)

  Returns the Mac UTI used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
    \fn QVariant QMacMime::convertToMime(const QString &mime, QList<QByteArray> data, QString flav)

    Returns \a data converted from Mac UTI \a flav to MIME type \a
    mime.

    Note that Mac flavors must all be self-terminating. The input \a
    data may contain trailing data.

    All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacMime::convertFromMime(const QString &mime, QVariant data, QString flav)

  Returns \a data converted from MIME type \a mime
    to Mac UTI \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

