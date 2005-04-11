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

#ifndef QT_NO_MIME

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

#include "qimagewriter.h"
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
            and supported by QTextDrag.
    \i kScrapFlavorTypeText - converted to "text/plain;charset=system" or "text/plain"
            and supported by QTextDrag.
    \i kScrapFlavorTypePicture - converted to "image/format", where format is
                a \link QImage::outputFormats() Qt image format\endlink,
            and supported by QImageDrag.
    \i kDragFlavorTypeHFS - converted to "text/uri-list",
            and supported by QUriDrag.
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
    QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav);
    QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav);
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
            qWarning("That shouldn't happen!");
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
            qWarning("This cannot really happen!!!");
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
                qWarning("Failure to open mime resources %s -- %s", library_file.fileName().toLatin1().constData(),
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
            qWarning("That shouldn't happen!");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            if(!library_file.isOpen()) {
                if(!library_file.open(QIODevice::WriteOnly)) {
                    qWarning("Failure to open %s -- %s", library_file.fileName().toLatin1().constData(),
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

QByteArray QMacMimeAnyMime::convertToMime(QList<QByteArray> data, const QString &, int)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeAnyMime::convertFromMime(QByteArray data, const QString &, int)
{
    QList<QByteArray> ret;
    ret.append(data);
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
    QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav);
    QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav);
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
        return kScrapFlavorTypeText;
    int i = mime.indexOf("charset=");
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
        return QString("text/plain");
    else if(flav == kScrapFlavorTypeUnicode)
        return QString("text/plain;charset=ISO-10646-UCS-2");
    return QString();
}

bool QMacMimeText::canConvert(const QString &mime, int flav)
{
    return flav && flavorFor(mime) == flav;
}

QByteArray QMacMimeText::convertToMime(QList<QByteArray> data, const QString &, int)
{
    if(data.count() > 1)
        qWarning("QMacMimeText: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeText::convertFromMime(QByteArray data, const QString &, int)
{
    QList<QByteArray> ret;
    ret.append(data);
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
    QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav);
    QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav);
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
    if(mime.startsWith(QLatin1String("image/"))) {
        QList<QByteArray> ofmts = QImageWriter::supportedImageFormats();
        for (int i = 0; i < ofmts.count(); ++i) {
            if (!qstricmp(ofmts.at(i), mime.mid(6).toLatin1()))
                return kScrapFlavorTypePicture;
        }
    }
    return 0;
}

QString QMacMimeImage::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypePicture)
        return QString("image/png");
    return QString();
}

bool QMacMimeImage::canConvert(const QString &mime, int flav)
{
    if(flav == kScrapFlavorTypePicture && mime.startsWith(QLatin1String("image/"))) {
        QList<QByteArray> ofmts = QImageWriter::supportedImageFormats();
        for (int i = 0; i < ofmts.count(); ++i) {
            if (!qstricmp(ofmts.at(i), mime.mid(6).toLatin1()))
                return true;
        }
    }
    return false;
}

QByteArray QMacMimeImage::convertToMime(QList<QByteArray> data, const QString &mime, int flav)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    QByteArray ret;
    if(!mime.startsWith(QLatin1String("image/")) || flav != kScrapFlavorTypePicture)
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
        QBuffer iod(&ret);
        iod.open(QIODevice::WriteOnly);
        QImage img = px.toImage();
        QImageWriter writer(&iod, mime.mid(6).toUpper().toLatin1());
        if(writer.write(img))
            iod.close();
    }
    DisposeHandle((Handle)pic);
    return ret;
}

QList<QByteArray> QMacMimeImage::convertFromMime(QByteArray data, const QString &mime, int flav)
{
    QList<QByteArray> ret;
    if(!mime.startsWith(QLatin1String("image/")) || flav != kScrapFlavorTypePicture)
        return ret;
    QPixmap px;
    {
        QImage img;
        img.loadFromData((unsigned char*)data.data(),data.size());
        if (img.isNull())
            return ret;
        px = QPixmap::fromImage(img);
    }
#if 1
    OpenCPicParams pic_params;
    pic_params.version = -2; // Version field is always -2
    SetRect(&pic_params.srcRect, 0, 0, px.width(), px.height());
    pic_params.hRes = pic_params.vRes = 0x00480000; // 72 dpi
    PicHandle pic = OpenCPicture(&pic_params);
    {
	GWorldPtr world;
	GetGWorld(&world, 0);
        ClipRect(&pic_params.srcRect);
	CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.macQDHandle()), GetPortBitMapForCopyBits((GWorldPtr)world),
                 &pic_params.srcRect, &pic_params.srcRect, srcCopy, 0);
    }
#else
    Rect r; SetRect(&r, 0, 0, px.width(), px.height());
    PicHandle pic = OpenPicture(&r);
    {
        GWorldPtr world;
        GDHandle handle;
        GetGWorld(&world, &handle);
        CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.macQDHandle()),
                 GetPortBitMapForCopyBits((GWorldPtr)world), &r, &r, srcCopy, 0);
    }
#endif
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
    QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav);
    QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav);
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

QByteArray QMacMimeFileUri::convertToMime(QList<QByteArray> data, const QString &mime, int flav)
{
    if(mime != QLatin1String("text/uri-list") || flav != typeFileURL)
        return QByteArray();
    int done = 0;
    QByteArray ret;
    for(QList<QByteArray>::const_iterator it = data.constBegin(); it != data.constEnd(); ++it) {
        QByteArray tmp_str(*it);
        if(tmp_str.left(17) == QLatin1String("file://localhost/")) //mac encodes a differently
            tmp_str = "file:///" + tmp_str.mid(17);
        int l = tmp_str.length();
        ret.resize(ret.size()+(l+2));
        memcpy(ret.data()+done,tmp_str.data(),l);
        memcpy(ret.data()+l+done,"\r\n",2);
        done += l + 2;
    }
    return ret;
}

QList<QByteArray> QMacMimeFileUri::convertFromMime(QByteArray data, const QString &mime, int flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("text/uri-list") || flav != typeFileURL)
        return ret;
    int len = 0;
    char *buffer = (char *)malloc(data.size());
    for(int i = 0; i < data.size(); i++) {
        if(data[i] == '\r' && i < data.size()-1 && data[i+1] == '\n')
            break;
        buffer[len++] = data[i];
    }
    if(!qstrncmp(buffer, "file:///", 8)) { //Mac likes localhost to be in it!
        QByteArray ar;
        ar.resize(len + 9);
        memcpy(ar.data(), buffer, 7);
        memcpy(ar.data()+7, "localhost", 9);
        memcpy(ar.data()+16, buffer + 7, len - 7);
        ret.append(ar);
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
    QByteArray convertToMime(QList<QByteArray> data, const QString &mime, int flav);
    QList<QByteArray> convertFromMime(QByteArray data, const QString &mime, int flav);
};

int QMacMimeHFSUri::countFlavors()
{
    return 1;
}

QString QMacMimeHFSUri::convertorName()
{
    return "HFSUri";
}

int QMacMimeHFSUri::flavor(int)
{
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
    if(flav == kDragFlavorTypeHFS)
        return QString("text/uri-list");
    return QString();
}

bool QMacMimeHFSUri::canConvert(const QString &mime, int flav)
{
    if(mime == QLatin1String("text/uri-list"))
        return flav == kDragFlavorTypeHFS;
    return false;
}

QByteArray QMacMimeHFSUri::convertToMime(QList<QByteArray> data, const QString &mime, int flav)
{
    if(mime != QLatin1String("text/uri-list") || flav != kDragFlavorTypeHFS)
        return QByteArray();
    int done = 0;
    QByteArray ret;
    char *buffer = (char *)malloc(1024);
    for(QList<QByteArray>::const_iterator it = data.constBegin(); it != data.constEnd(); ++it) {
        FSRef fsref;
        HFSFlavor *hfs = (HFSFlavor *)(*it).data();
        FSpMakeFSRef(&hfs->fileSpec, &fsref);
        FSRefMakePath(&fsref, (UInt8 *)buffer, 1024);

        QByteArray s = QVariant(QUrl(QString::fromUtf8((const char *)buffer))).toByteArray();
        const int l = s.size();
        //now encode them to be handled by quridrag
        ret.resize(ret.size()+(l+2));
        memcpy(ret.data()+done,s,l);
        memcpy(ret.data()+l+done,"\r\n",2);
        done += l + 2;
    }
    free(buffer);
    return ret;
}

QList<QByteArray> QMacMimeHFSUri::convertFromMime(QByteArray data, const QString &mime, int flav)
{
    QList<QByteArray> ret;
    if(mime != QLatin1String("text/uri-list") || flav != kDragFlavorTypeHFS)
        return ret;
    HFSFlavor hfs;
    hfs.fileType = 'TEXT';
    hfs.fileCreator = qt_mac_mime_type;
    hfs.fdFlags = 0;
    if(qt_mac_create_fsspec(QString(data), &hfs.fileSpec) == noErr)
        ret.append(data);
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
    // return nothing for illegal requests
    if(!flav)
        return 0;

    MimeList *mimes = globalMimeList();
    for(MimeList::const_iterator it = mimes->constBegin(); it != mimes->constEnd(); ++it) {
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
  \fn QByteArray QMacMime::convertToMime(QList<QByteArray> data, const QString &mime, int flav)

  Returns \a data converted from Mac flavor \a flav to MIME type \a
    mime.

  Note that Mac flavors must all be self-terminating.  The input \a
  data may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacMime::convertFromMime(QByteArray data, const QString &mime, int flav)

  Returns \a data converted from MIME type \a mime
    to Mac flavor \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

#endif // QT_NO_MIME
