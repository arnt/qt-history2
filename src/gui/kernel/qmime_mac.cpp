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
    \i kDragFlavorTypeHFS - converted to "text/uri-list"
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

ScrapFlavorType qt_mac_mime_type = 'CUTE';
class QMacMimeAnyMime : public QMacMime {
private:
#ifdef USE_INTERNET_CONFIG
    ICInstance internet_config;
    long mime_registry_version;
#else
    int current_max;
    QFile library_file;
    QDateTime mime_registry_loaded;
#endif
    QMap<QString, int> mime_registry;
    int registerMimeType(const QString &mime);
    bool loadMimeRegistry();

public:
    QMacMimeAnyMime() : QMacMime(MIME_QT_CONVERTOR|MIME_ALL) {
#ifdef USE_INTERNET_CONFIG
        internet_config = 0;
#else
        current_max = 'QT00';
#endif
    }
    ~QMacMimeAnyMime() {
#ifdef USE_INTERNET_CONFIG
        if(internet_config)
            ICStop(internet_config);
#endif
    }
    int countFlavors();
    QString convertorName();
    int flavor(int index);
    int flavorFor(const QString &mime);
    QString mimeFor(int flav);
    bool canConvert(const QString &mime, int flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav);
};

#ifdef USE_INTERNET_CONFIG
bool QMacMimeAnyMime::loadMimeRegistry()
{
    if(!internet_config) { //need to start
        ICStart(&internet_config, qt_mac_mime_type);
        ICGetSeed(internet_config, &mime_registry_version);
    } else { //do we need to do anything?
        long mt;
        ICGetSeed(internet_config, &mt);
        if(mt == mime_registry_version)
            return true;
        mime_registry_version = mt;
        mime_registry.clear();
    }

    //start parsing
    ICBegin(internet_config, icReadOnlyPerm);
    Handle hdl = NewHandle(0);
    ICAttr attr = kICAttrNoChange;
    Str255 mapping_name;
    qt_mac_to_pascal_string("Mapping", mapping_name);
    ICFindPrefHandle(internet_config, mapping_name, &attr, hdl);

    //get count
    long count;
    ICCountMapEntries(internet_config, hdl, &count);

    //enumerate all entries
    ICMapEntry entry;
    for(int i = 0; i < count; i++) {
        long pos;
        ICGetIndMapEntry(internet_config, hdl, i, &pos, &entry);
        QString mime = qt_mac_from_pascal_string(entry.MIMEType);
        if(!mime.isEmpty())
            mime_registry.insert(mime, entry.fileType);
    }

    //cleanup
    DisposeHandle(hdl);
    ICEnd(internet_config);
    return true;
}

inline static void qt_mac_copy_to_str255(const QString &qstr, unsigned char *pstr)
{
    int length = qstr.length();
    Q_ASSERT(length < 255);
    pstr[0] = (uchar)length;
    memcpy(pstr+1, qstr.toLatin1(), length);
}

int QMacMimeAnyMime::registerMimeType(const QString &mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("QMime: Internal error");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            for(int ret = 'QT00';  true; ret++) {
                bool found = false;
                for(QMap::const_iterator<QString, int> it = mime_registry.constBegin();
                    it != mime_registry.constEnd(); ++it) {
                    if(it.data() == ret) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    //create the entry
                    ICMapEntry entry;
                    memset(&entry, '\0', sizeof(entry));
                    entry.fixedLength = kICMapFixedLength;
                    entry.fileType = ret;
                    entry.postCreator = entry.fileCreator = qt_mac_mime_type;
                    entry.flags = kICMapBinaryMask;
                    qt_mac_copy_to_str255("Qt Library", entry.creatorAppName);
                    qt_mac_copy_to_str255("Qt Library", entry.postAppName);
                    qt_mac_copy_to_str255(mime, entry.MIMEType);
                    qt_mac_copy_to_str255(QString("Qt Library mime mapping (%1)").arg(mime), entry.postAppName);

                    //insert into the config
                    Str255 mapping_name;
                    qt_mac_to_pascal_string("Mapping", mapping_name);
                    ICBegin(internet_config, icReadWritePerm);
                    Handle hdl = NewHandle(0);
                    ICAttr attr;
                    ICFindPrefHandle(internet_config, mapping_name, &attr, hdl);

                    ICAddMapEntry(internet_config, hdl, &entry);
                    ICSetPrefHandle(internet_config, mapping_name, attr, hdl);
                    mime_registry.insert(mime, ret);

                    //cleanup
                    ICEnd(internet_config);
                    ICGetSeed(internet_config, &mime_registry_version); //get new seed since we manually update
                    DisposeHandle(hdl);
                    return ret;
                }
            }
            qWarning("QMime: Internal error");
            return 0;
        }
    }
    return mime_registry[mime];
}
#else
bool openMimeRegistry(bool global, QIODevice::OpenMode mode, QFile &file)
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
void loadMimeRegistry(QFile &file, QMap<QString, int> &registry, int &max)
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
bool QMacMimeAnyMime::loadMimeRegistry()
{
    if(!library_file.isOpen()) {
        if(!openMimeRegistry(true, QIODevice::ReadWrite, library_file)) {
            QFile global;
            if(openMimeRegistry(true, QIODevice::ReadOnly, global)) {
                ::loadMimeRegistry(global, mime_registry, current_max);
                global.close();
            }
            if(!openMimeRegistry(false, QIODevice::ReadWrite, library_file)) {
                qWarning("QMacMimeAnyMime: Failure to open mime resources %s -- %s", library_file.fileName().toLatin1().constData(),
                         library_file.errorString().toLatin1().constData());
                return false;
            }
        }
    }

    QFileInfo fi(library_file);
    if(!mime_registry_loaded.isNull() && mime_registry_loaded == fi.lastModified())
        return true;
    mime_registry_loaded = fi.lastModified();
    ::loadMimeRegistry(library_file, mime_registry, current_max);
    return true;
}

int QMacMimeAnyMime::registerMimeType(const QString &mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("QMacMimeAnyMime: Internal error");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            if(!library_file.isOpen()) {
                if(!library_file.open(QIODevice::WriteOnly)) {
                    qWarning("QMacMimeAnyMime: Failure to open %s -- %s", library_file.fileName().toLatin1().constData(),
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

#endif

int QMacMimeAnyMime::countFlavors()
{
    loadMimeRegistry();
    return mime_registry.count();
}

QString QMacMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

int QMacMimeAnyMime::flavor(int index)
{
    loadMimeRegistry();
    int i = 0;
    for(QMap<QString, int>::const_iterator it = mime_registry.constBegin();
         it != mime_registry.constEnd(); ++it, ++i) {
        if(i == index)
            return it.value();
    }
    return 0;
}

int QMacMimeAnyMime::flavorFor(const QString &mime)
{
    return registerMimeType(mime);
}

QString QMacMimeAnyMime::mimeFor(int flav)
{
    loadMimeRegistry();
    for(QMap<QString, int>::const_iterator it = mime_registry.constBegin();
        it != mime_registry.constEnd(); ++it) {
        if(it.value() == flav)
            return it.key().toLatin1();
    }
    return QString();
}

bool QMacMimeAnyMime::canConvert(const QString &mime, int flav)
{
    loadMimeRegistry();
    if(mime_registry.contains(mime) && mime_registry[mime] == flav)
        return true;
    return false;
}

QVariant QMacMimeAnyMime::convertToMime(const QString &mime, QList<QByteArray> data, int)
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

QList<QByteArray> QMacMimeAnyMime::convertFromMime(const QString &mime, QVariant data, int)
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
    int countFlavors();
    QString convertorName();
    int flavor(int index);
    int flavorFor(const QString &mime);
    QString mimeFor(int flav);
    bool canConvert(const QString &mime, int flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav);
};

int QMacMimeText::countFlavors()
{
    return 2;
}

QString QMacMimeText::convertorName()
{
    return "Text";
}

int QMacMimeText::flavor(int index)
{
    if(index == 0)
        return kScrapFlavorTypeUnicode;
    return kScrapFlavorTypeText;
}

int QMacMimeText::flavorFor(const QString &mime)
{
    if(mime == QLatin1String("text/plain"))
        return kScrapFlavorTypeUnicode;
    int i = mime.indexOf(QLatin1String("charset="));
    if(i >= 0) {
        QString cs(mime.mid(i+8));
        i = cs.indexOf(";");
        if(i>=0)
            cs = cs.left(i);
        if(cs == QLatin1String("system"))
            return kScrapFlavorTypeText;
        else if(cs == QLatin1String("ISO-10646-UCS-2") ||
                cs == QLatin1String("utf16"))
            return kScrapFlavorTypeUnicode;
    }
    return 0;
}

QString QMacMimeText::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypeText)
        return QLatin1String("text/plain");
    else if(flav == kScrapFlavorTypeUnicode)
        return QLatin1String("text/plain;charset=ISO-10646-UCS-2");
    return QString();
}

bool QMacMimeText::canConvert(const QString &mime, int flav)
{
    return flav && flavorFor(mime) == flav;
}

QVariant QMacMimeText::convertToMime(const QString &mimetype, QList<QByteArray> data, int flavor)
{
    if(data.count() > 1)
        qWarning("QMacMimeText: Cannot handle multiple member data");
    const QByteArray &firstData = data.first();
    // I can only handle two types (system and unicode) so deal with them that way
    QVariant ret;
    switch (flavor) {
    case kScrapFlavorTypeText: {
        QCFString str(CFStringCreateWithBytes(kCFAllocatorDefault,
                                             reinterpret_cast<const UInt8 *>(firstData.constData()),
                                             firstData.size(), CFStringGetSystemEncoding(), false));
        ret = QString(str);
        break; }
    case kScrapFlavorTypeUnicode:
        ret = QString::fromUtf16(reinterpret_cast<const ushort *>(firstData.constData()),
                                 firstData.size() / sizeof(ushort));
        break;
    default:
        qWarning("QMime::convertToMime: unhandled mimetype: %s", qPrintable(mimetype));
        break;
    }
    return ret;
}

QList<QByteArray> QMacMimeText::convertFromMime(const QString &, QVariant data, int flavor)
{
    QList<QByteArray> ret;
    QString string = data.toString();
    if(flavor == kScrapFlavorTypeText)
        ret.append(string.toLatin1());
    else if(flavor == kScrapFlavorTypeUnicode)
        ret.append(QByteArray((char*)string.utf16(), string.length()*2));
    return ret;
}


class QMacMimeImage : public QMacMime {
public:
    QMacMimeImage() : QMacMime(MIME_ALL) { }
    int countFlavors();
    QString convertorName();
    int flavor(int index);
    int flavorFor(const QString &mime);
    QString mimeFor(int flav);
    bool canConvert(const QString &mime, int flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav);
};

int QMacMimeImage::countFlavors()
{
    return 1;
}

QString QMacMimeImage::convertorName()
{
    return "Image";
}

int QMacMimeImage::flavor(int)
{
    return kScrapFlavorTypePicture;
}

int QMacMimeImage::flavorFor(const QString &mime)
{
    if(mime.startsWith(QLatin1String("application/x-qt-image")))
        return kScrapFlavorTypePicture;
    return 0;
}

QString QMacMimeImage::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypePicture)
        return QString("application/x-qt-image");
    return QString();
}

bool QMacMimeImage::canConvert(const QString &mime, int flav)
{
    if(flav == kScrapFlavorTypePicture && mime == QLatin1String("application/x-qt-image"))
        return true;
    return false;
}

QVariant QMacMimeImage::convertToMime(const QString &mime, QList<QByteArray> data, int flav)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: Cannot handle multiple member data");
    QVariant ret;
    if(mime != QLatin1String("application/x-qt-image") || flav != kScrapFlavorTypePicture)
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

QList<QByteArray> QMacMimeImage::convertFromMime(const QString &mime, QVariant variant, int flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("application/x-qt-image") || flav != kScrapFlavorTypePicture)
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
    int countFlavors();
    QString convertorName();
    int flavor(int index);
    int flavorFor(const QString &mime);
    QString mimeFor(int flav);
    bool canConvert(const QString &mime, int flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav);
};

int QMacMimeFileUri::countFlavors()
{
    return 1;
}

QString QMacMimeFileUri::convertorName()
{
    return "FileURL";
}

int QMacMimeFileUri::flavor(int)
{
    return typeFileURL;
}

int QMacMimeFileUri::flavorFor(const QString &mime)
{
    if(mime != QLatin1String("text/uri-list"))
        return 0;
    return (int)typeFileURL;
}

QString QMacMimeFileUri::mimeFor(int flav)
{
    if(flav == typeFileURL)
        return QString("text/uri-list");
    return QString();
}

bool QMacMimeFileUri::canConvert(const QString &mime, int flav)
{
    if(mime == QLatin1String("text/uri-list"))
        return flav == typeFileURL;
    return false;
}

QVariant QMacMimeFileUri::convertToMime(const QString &mime, QList<QByteArray> data, int flav)
{
    if(mime != QLatin1String("text/uri-list") || flav != typeFileURL)
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

QList<QByteArray> QMacMimeFileUri::convertFromMime(const QString &mime, QVariant data, int flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("text/uri-list") || flav != typeFileURL)
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
        ret.append(uri.toLatin1());
    }
    return ret;
}

class QMacMimeHFSUri : public QMacMime {
public:
    QMacMimeHFSUri() : QMacMime(MIME_DND) { }
    int countFlavors();
    QString convertorName();
    int flavor(int index);
    int flavorFor(const QString &mime);
    QString mimeFor(int flav);
    bool canConvert(const QString &mime, int flav);
    QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav);
    QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav);
};

int QMacMimeHFSUri::countFlavors()
{
    return 2;
}

QString QMacMimeHFSUri::convertorName()
{
    return "HFSUri";
}

int QMacMimeHFSUri::flavor(int flav)
{
    if(flav == 0)
        return kDragFlavorTypePromiseHFS;
    return kDragFlavorTypeHFS;
}

int QMacMimeHFSUri::flavorFor(const QString &mime)
{
    if(mime != QLatin1String("text/uri-list"))
        return 0;
    return (int)kDragFlavorTypeHFS;
}

QString QMacMimeHFSUri::mimeFor(int flav)
{
    if(flav == kDragFlavorTypeHFS || flav == kDragFlavorTypePromiseHFS)
        return QString("text/uri-list");
    return QString();
}

bool QMacMimeHFSUri::canConvert(const QString &mime, int flav)
{
    if(mime == QLatin1String("text/uri-list"))
        return flav == kDragFlavorTypeHFS || flav == kDragFlavorTypePromiseHFS;
    return false;
}

QVariant QMacMimeHFSUri::convertToMime(const QString &mime, QList<QByteArray> data, int flav)
{
    if(mime != QLatin1String("text/uri-list") ||
       (flav != kDragFlavorTypeHFS && flav != kDragFlavorTypePromiseHFS))
        return QByteArray();
    QList<QVariant> ret;
    char *buffer = (char*)malloc(1024);
    for(int i = 0; i < data.size(); ++i) {
        FSRef fsref;
        HFSFlavor *hfs = (HFSFlavor *)data.at(i).data();
        FSpMakeFSRef(&hfs->fileSpec, &fsref);
        FSRefMakePath(&fsref, (UInt8 *)buffer, 1024);
        ret.append(QUrl(QString::fromUtf8((const char *)buffer)));
    }
    free(buffer);
    return QVariant(ret);
}

QList<QByteArray> QMacMimeHFSUri::convertFromMime(const QString &mime, QVariant data, int flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("text/uri-list") ||
       (flav != kDragFlavorTypeHFS && flav != kDragFlavorTypePromiseHFS))
        return ret;
    QList<QVariant> urls = data.toList();
    for(int i = 0; i < urls.size(); ++i) {
#if 0
        QUrl url = urls.at(i).toUrl();
        QString uri;
        if(url.scheme().isEmpty())
            uri = QUrl::fromLocalFile(url.toString()).toString();
        else
            uri = url.toString();
#else
        QString uri = urls.at(i).toUrl().toString();
#endif

        HFSFlavor hfs;
        hfs.fileType = 'TEXT';
        hfs.fileCreator = qt_mac_mime_type;
        hfs.fdFlags = 0;
        if(qt_mac_create_fsspec(uri, &hfs.fileSpec) == noErr)
            ret.append(uri.toLatin1());
    }
    return ret;
}

/*!
  \internal

  This is an internal function.
*/
void QMacMime::initialize()
{
    if(globalMimeList()->isEmpty()) {
        qAddPostRoutine(cleanup_mimes);
        new QMacMimeImage;
        new QMacMimeText;
        new QMacMimeFileUri;
        new QMacMimeHFSUri;
        new QMacMimeAnyMime;
    }
}

/*!
  Returns the most-recently created QMacMime of type \a t that can convert
  between the \a mime and \a flav formats.  Returns 0 if no such convertor
  exists.
*/
QMacMime*
QMacMime::convertor(QMacMimeType t, const QString &mime, int flav)
{
    if(!flav)
        return 0;

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
        if(((*it)->type & t) && (*it)->canConvert(mime,flav))
            return (*it);
    }
    return 0;
}

/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
QString QMacMime::flavorToMime(QMacMimeType t, int flav)
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
QList<QMacMime*> QMacMime::all(QMacMimeType t)
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
  \fn int QMacMime::countFlavors()

  Returns the number of Mac flavors supported by this convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::flavor(int index)

  Returns the Mac flavor supported by this convertor that is
  ordinarily at position \a index. This means that flavor(0) returns
  the first Mac flavor supported, and flavor(countFlavors()-1) returns
  the last. If \a index is out of range the return value is undefined.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QMacMime::canConvert(const QString &mime, int flav)

  Returns true if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns false.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QString QMacMime::mimeFor(int flav)

  Returns the MIME type used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::flavorFor(const QString &mime)

  Returns the Mac flavor used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
    \fn QVariant QMacMime::convertToMime(const QString &mime, QList<QByteArray> data, int flav)

    Returns \a data converted from Mac flavor \a flav to MIME type \a
    mime.

    Note that Mac flavors must all be self-terminating. The input \a
    data may contain trailing data.

    All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacMime::convertFromMime(const QString &mime, QVariant data, int flav)

  Returns \a data converted from MIME type \a mime
    to Mac flavor \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

