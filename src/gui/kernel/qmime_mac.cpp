/****************************************************************************
**
** Implementation of Mac flavor <-> clipboard converters.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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

#include "qstrlist.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qdatastream.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qmap.h"
#include "qt_mac.h"

static QList<QMacMime*> mimes;

static void cleanup_mimes()
{
    while (!mimes.isEmpty())
        delete mimes.takeFirst();
}

//functions
QByteArray pstring2qstring(const unsigned char *); //qglobal.cpp
unsigned char * p_str(const QString &s); //qglobal.cpp
OSErr qt_mac_create_fsspec(const QString &path, FSSpec *spec); //qglobal_mac.cpp

/*!
  \class QMacMime
  \brief The QMacMime class maps open-standard MIME to Mac flavors.
  \ingroup io
  \ingroup draganddrop
  \ingroup misc

  Qt's drag-and-drop and clipboard facilities use the MIME standard.
  On X11, this maps trivially to the Xdnd protocol, but on Mac
  although some applications use MIME types to describe clipboard
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
    mimes.append(this);
}

/*!
  Destroys a conversion object, removing it from the global
  list of available convertors.
*/
QMacMime::~QMacMime()
{
    if(!QApplication::closingDown())
        mimes.removeAll(this);
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
    int registerMimeType(const char *mime);
    bool loadMimeRegistry();

public:
    QMacMimeAnyMime() : QMacMime(MIME_QT_CONVERTOR|MIME_ALL) {
#ifdef USE_INTERNET_CONFIG
        internet_config = NULL;
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
    int                countFlavors();
    const char* convertorName();
    int                flavor(int index);
    int                flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool        canConvert(const char* mime, int flav);
    QByteArray                   convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
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
    ICFindPrefHandle(internet_config, p_str("Mapping"), &attr, hdl);

    //get count
    long count;
    ICCountMapEntries(internet_config, hdl, &count);

    //enumerate all entries
    ICMapEntry entry;
    for(int i = 0; i < count; i++) {
        long pos;
        ICGetIndMapEntry(internet_config, hdl, i, &pos, &entry);
        QString mime = pstring2qstring(entry.MIMEType);
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
    memcpy(pstr+1, qstr.latin1(), length);
}

int QMacMimeAnyMime::registerMimeType(const char *mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("That shouldn't happen!");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            for(int ret = 'QT00';  true; ret++) {
                bool found = false;
                for(QMapIterator<QString, int> it = mime_registry.begin(); it != mime_registry.end(); ++it) {
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
                    ICBegin(internet_config, icReadWritePerm);
                    Handle hdl = NewHandle(0);
                    ICAttr attr;
                    ICFindPrefHandle(internet_config, p_str("Mapping"), &attr, hdl);

                    ICAddMapEntry(internet_config, hdl, &entry);
                    ICSetPrefHandle(internet_config, p_str("Mapping"), attr, hdl);
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
bool QMacMimeAnyMime::loadMimeRegistry()
{
    const QString mimeTypeDir = "/Library/Qt";
    if(!library_file.isOpen()) {
        if(!QFile::exists(mimeTypeDir)) {
            // Do it with a system call as I don't see much worth in
            // doing it with QDir since we have to chmod anyway.
            bool success = ::mkdir(mimeTypeDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
            if (success)
                success = ::chmod(mimeTypeDir.local8Bit(), S_IRUSR | S_IWUSR | S_IXUSR
                                  | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) == 0;
            if (!success) {
                qWarning("Problem creating %s: %s", mimeTypeDir.latin1(), strerror(errno));
                return false;
            }
        }
        library_file.setName(mimeTypeDir + "/.mime_types");
        if (!library_file.exists()) {
            // Create the file and chmod it so that everyone can write to it.
            int fd = ::open(library_file.name().local8Bit(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            bool success = fd != -1;
            if (success)
                success = ::fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == 0;
            int olderrno = errno;
            if (fd != -1)
                ::close(fd);
            if (!success) {
                qWarning("Problem creating %s: %s", library_file.name().latin1(), strerror(olderrno));
                return false;
            }
        }
        if(!library_file.open(IO_ReadWrite)) {
            qWarning("Failure to open %s -- %s", library_file.name().latin1(),
                     library_file.errorString().latin1());
            return false;
        }
        library_file.reset();
    }
    QFileInfo fi(library_file);
    if(!mime_registry_loaded.isNull() && mime_registry_loaded == fi.lastModified())
        return true;
    mime_registry_loaded = fi.lastModified();
    QTextStream stream(&library_file);
    while(!stream.atEnd()) {
        QString mime = stream.readLine();
        int mactype = stream.readLine().toInt();
        if(mactype > current_max)
            current_max = mactype;
        mime_registry.insert(mime, mactype);
    }
    return true;
}

int QMacMimeAnyMime::registerMimeType(const char *mime)
{
    if(!mime_registry.contains(mime)) {
        if(!loadMimeRegistry()) {
            qWarning("That shouldn't happen!");
            return 0;
        }
        if(!mime_registry.contains(mime)) {
            if(!library_file.isOpen()) {
                if(!library_file.open(IO_WriteOnly)) {
                    qWarning("Failure to open %s -- %s", library_file.name().latin1(),
                             library_file.errorString().latin1());
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

const char* QMacMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

int QMacMimeAnyMime::flavor(int index)
{
    loadMimeRegistry();
    int i = 0;
    for(QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it, ++i) {
        if(i == index)
            return it.value();
    }
    return 0;
}

int QMacMimeAnyMime::flavorFor(const char* mime)
{
    return registerMimeType(mime);
}

const char* QMacMimeAnyMime::mimeFor(int flav)
{
    loadMimeRegistry();
    for(QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it) {
        if(it.value() == flav)
            return it.key().latin1();
    }
    return NULL;
}

bool QMacMimeAnyMime::canConvert(const char* mime, int flav)
{
    loadMimeRegistry();
    if(mime_registry.contains(mime) && mime_registry[mime] == flav)
        return true;
    return false;
}

QByteArray QMacMimeAnyMime::convertToMime(QList<QByteArray> data, const char* , int)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeAnyMime::convertFromMime(QByteArray data, const char* , int)
{
    QList<QByteArray> ret;
    ret.append(data);
    return ret;
}


class QMacMimeText : public QMacMime {
public:
    QMacMimeText() : QMacMime(MIME_ALL) { }
    int                countFlavors();
    const char* convertorName();
    int                flavor(int index);
    int                flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool        canConvert(const char* mime, int flav);
    QByteArray                   convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeText::countFlavors()
{
    return 2;
}

const char* QMacMimeText::convertorName()
{
    return "Text";
}

int QMacMimeText::flavor(int index)
{
    if(index == 0)
        return kScrapFlavorTypeUnicode;
    return kScrapFlavorTypeText;
}

int QMacMimeText::flavorFor(const char* mime)
{
    if(!qstricmp(mime, "text/plain"))
        return kScrapFlavorTypeText;
    QByteArray m(mime);
    int i = m.indexOf("charset=");
    if(i >= 0) {
        QByteArray cs(m.data()+i+8);
        i = cs.indexOf(";");
        if(i>=0)
            cs = cs.left(i);
        if(cs == "system")
            return kScrapFlavorTypeText;
        else if(cs == "ISO-10646-UCS-2" || cs == "utf16")
            return kScrapFlavorTypeUnicode;
    }
    return 0;
}

const char* QMacMimeText::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypeText)
        return "text/plain";
    else if(flav == kScrapFlavorTypeUnicode)
        return "text/plain;charset=ISO-10646-UCS-2";
    return NULL;
}

bool QMacMimeText::canConvert(const char* mime, int flav)
{
    return flav && flavorFor(mime) == flav;
}

QByteArray QMacMimeText::convertToMime(QList<QByteArray> data, const char*, int)
{
    if(data.count() > 1)
        qWarning("QMacMimeText: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeText::convertFromMime(QByteArray data, const char*, int)
{
    QList<QByteArray> ret;
    ret.append(data);
    return ret;
}


class QMacMimeImage : public QMacMime {
public:
    QMacMimeImage() : QMacMime(MIME_ALL) { }
    int                countFlavors();
    const char* convertorName();
    int                flavor(int index);
    int                flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool        canConvert(const char* mime, int flav);
    QByteArray                   convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeImage::countFlavors()
{
    return 1;
}

const char* QMacMimeImage::convertorName()
{
    return "Image";
}

int QMacMimeImage::flavor(int)
{
    return kScrapFlavorTypePicture;
}

int QMacMimeImage::flavorFor(const char* mime)
{
    if(!qstrnicmp(mime,"image/",5)) {
        QList<QByteArray> ofmts = QImage::outputFormats();
        for (int i = 0; i < ofmts.count(); ++i) {
            if (!qstricmp(ofmts.at(i), mime + 6))
                return kScrapFlavorTypePicture;
        }
    }
    return 0;
}

const char* QMacMimeImage::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypePicture)
        return "image/png";
    return 0;
}

bool QMacMimeImage::canConvert(const char* mime, int flav)
{
    if(flav == kScrapFlavorTypePicture && !qstrnicmp(mime,"image/",5)) {
        QList<QByteArray> ofmts = QImage::outputFormats();
        for (int i = 0; i < ofmts.count(); ++i) {
            if (!qstricmp(ofmts.at(i), mime + 6))
                return true;
        }
    }
    return false;
}

QByteArray QMacMimeImage::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(data.count() > 1)
        qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    QByteArray ret;
    if(qstrnicmp(mime,"image/",6) || flav != kScrapFlavorTypePicture)
        return ret;
    QByteArray &a = data.first();
    PicHandle pic = (PicHandle)NewHandle(a.size());
    memcpy(*pic, a.data(), a.size());
    PictInfo pinfo;
    if(GetPictInfo(pic, &pinfo, 0, 0, 0, 0) == noErr) {
        QPixmap px(pinfo.sourceRect.right - pinfo.sourceRect.left,
                   pinfo.sourceRect.bottom - pinfo.sourceRect.top, 32);
        {
            Rect r; SetRect(&r, 0, 0, px.width(), px.height());
            QMacSavedPortInfo pi(&px);
            DrawPicture(pic, &r);
        }
        QByteArray ofmt = mime+6;
        QBuffer iod(&ret);
        iod.open(IO_WriteOnly);
        QImage img = px.convertToImage();
        QImageIO iio(&iod, ofmt.toUpper());
        iio.setImage(img);
        if(iio.write())
            iod.close();
    }
    DisposeHandle((Handle)pic);
    return ret;
}

QList<QByteArray> QMacMimeImage::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstrnicmp(mime,"image/",6) || flav != kScrapFlavorTypePicture)
        return ret;
    QPixmap px;
    {
        QImage img;
        img.loadFromData((unsigned char*)data.data(),data.size());
        if (img.isNull())
            return ret;
        px = img;
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
	CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.handle()), GetPortBitMapForCopyBits((GWorldPtr)world), 
                 &pic_params.srcRect, &pic_params.srcRect, srcCopy, 0);
    }
#else
    Rect r; SetRect(&r, 0, 0, px.width(), px.height());
    PicHandle pic = OpenPicture(&r);
    {
        GWorldPtr world;
        GDHandle handle;
        GetGWorld(&world, &handle);
        CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.handle()),
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
    int                countFlavors();
    const char* convertorName();
    int                flavor(int index);
    int                flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool        canConvert(const char* mime, int flav);
    QByteArray                   convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeFileUri::countFlavors()
{
    return 1;
}

const char* QMacMimeFileUri::convertorName()
{
    return "FileURL";
}

int QMacMimeFileUri::flavor(int)
{
    return typeFileURL;
}

int QMacMimeFileUri::flavorFor(const char* mime)
{
    if(qstricmp(mime,"text/uri-list"))
        return 0;
    return (int)typeFileURL;
}

const char* QMacMimeFileUri::mimeFor(int flav)
{
    if(flav == typeFileURL)
        return "text/uri-list";
    return NULL;
}

bool QMacMimeFileUri::canConvert(const char* mime, int flav)
{
    if(!qstricmp(mime,"text/uri-list"))
        return flav == typeFileURL;
    return false;
}

QByteArray QMacMimeFileUri::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(qstricmp(mime,"text/uri-list") || flav != typeFileURL)
        return QByteArray();
    int done = 0;
    QByteArray ret;
    for(QList<QByteArray>::Iterator it = data.begin(); it != data.end(); ++it) {
        QByteArray tmp_str(*it);
        if(tmp_str.left(17) == "file://localhost/") //mac encodes a differently
            tmp_str = "file:///" + tmp_str.mid(17);
        int l = tmp_str.length();
        ret.resize(ret.size()+(l+2));
        memcpy(ret.data()+done,tmp_str.data(),l);
        memcpy(ret.data()+l+done,"\r\n",2);
        done += l + 2;
    }
    return ret;
}

QList<QByteArray> QMacMimeFileUri::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstricmp(mime,"text/uri-list") || flav != typeFileURL)
        return ret;
    int len = 0;
    char *buffer = (char *)malloc(data.size());
    for(int i = 0; i < data.size(); i++) {
        if(data[i] == '\r' && i < data.size()-1 && data[i+1] == '\n')
            break;
        buffer[len++] = data[i];
    }
    if(!qstrncmp(buffer, "file:///", 8)) { //Mac likes localhost to be in it!
        QByteArray ar(len + 9);
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
    int                countFlavors();
    const char* convertorName();
    int                flavor(int index);
    int                flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool        canConvert(const char* mime, int flav);
    QByteArray                   convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeHFSUri::countFlavors()
{
    return 1;
}

const char* QMacMimeHFSUri::convertorName()
{
    return "HFSUri";
}

int QMacMimeHFSUri::flavor(int)
{
    return kDragFlavorTypeHFS;
}

int QMacMimeHFSUri::flavorFor(const char* mime)
{
    if(qstricmp(mime,"text/uri-list"))
        return 0;
    return (int)kDragFlavorTypeHFS;
}

const char* QMacMimeHFSUri::mimeFor(int flav)
{
    if(flav == kDragFlavorTypeHFS)
        return "text/uri-list";
    return NULL;
}

bool QMacMimeHFSUri::canConvert(const char* mime, int flav)
{
    if(!qstricmp(mime,"text/uri-list"))
        return flav == kDragFlavorTypeHFS;
    return false;
}

QByteArray QMacMimeHFSUri::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(qstricmp(mime,"text/uri-list") || flav != kDragFlavorTypeHFS)
        return QByteArray();
    int done = 0;
    QByteArray ret;
    char *buffer = (char *)malloc(1024);
    for(QList<QByteArray>::Iterator it = data.begin(); it != data.end(); ++it) {
        FSRef fsref;
        HFSFlavor *hfs = (HFSFlavor *)(*it).data();
        FSpMakeFSRef(&hfs->fileSpec, &fsref);
        FSRefMakePath(&fsref, (UInt8 *)buffer, 1024);
        QByteArray s = QUriDrag::localFileToUri(QString::fromUtf8((const char *)buffer));
        //now encode them to be handled by quridrag
        int l = qstrlen(s);
        ret.resize(ret.size()+(l+2));
        memcpy(ret.data()+done,s,l);
        memcpy(ret.data()+l+done,"\r\n",2);
        done += l + 2;
    }
    free(buffer);
    return ret;
}

QList<QByteArray> QMacMimeHFSUri::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstricmp(mime,"text/uri-list") || flav != kDragFlavorTypeHFS)
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
    mimes.ensure_constructed();
    if(mimes.isEmpty()) {
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
QMacMime::convertor(QMacMimeType t, const char *mime, int flav)
{
    // return nothing for illegal requests
    if(!flav)
        return 0;

    for(QList<QMacMime *>::Iterator it = mimes.begin(); it != mimes.end(); ++it) {
        if(((*it)->type & t) && (*it)->canConvert(mime,flav))
            return (*it);
    }
    return 0;
}

/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
const char* QMacMime::flavorToMime(QMacMimeType t, int flav)
{
    for(QList<QMacMime *>::Iterator it = mimes.begin(); it != mimes.end(); ++it) {
        if((*it)->type & t)
            return (*it)->mimeFor(flav);
    }
    return NULL;
}

/*!
  Returns a list of all currently defined QMacMime objects of type \a t.
*/
QList<QMacMime*> QMacMime::all(QMacMimeType t)
{
    QList<QMacMime*> ret;
    for(QList<QMacMime *>::Iterator it = mimes.begin(); it != mimes.end(); ++it) {
        if((*it)->type & t)
            ret.append((*it));
    }
    return ret;
}

/*!
  \fn const char* QMacMime::convertorName()

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
  \fn bool QMacMime::canConvert(const char* mime, int flav)

  Returns true if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns false.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn const char* QMacMime::mimeFor(int flav)

  Returns the MIME type used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::flavorFor(const char* mime)

  Returns the Mac flavor used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QByteArray QMacMime::convertToMime(QList<QByteArray> data, const char* mime, int flav)

  Returns \a data converted from Mac flavor \a flav to MIME type \a
    mime.

  Note that Mac flavors must all be self-terminating.  The input \a
  data may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacMime::convertFromMime(QByteArray data, const char* mime, int flav)

  Returns \a data converted from MIME type \a mime
    to Mac flavor \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

#endif // QT_NO_MIME
